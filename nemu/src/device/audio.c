/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <SDL2/SDL.h>
#include <common.h>
#include <device/map.h>

enum {
    reg_freq,
    reg_channels,
    reg_samples,

    reg_sbuf_size,
    reg_init,
    reg_count,
    nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;
static SDL_AudioSpec desired;
static int count = 0;
static int left = 0;
static int right = 0;
static uint32_t full_len = 0;

static void audio_play(void *userdata, uint8_t *stream, int len) {
    // uint8_t *sbuf = (uint8_t *) (uintptr_t) CONFIG_SB_ADDR;
    int nwrite = len;
    if (count < nwrite)
        nwrite = count;
    for (int i = 0; i < nwrite; ++i) {
        stream[i] = 255;
        // stream[i] = sbuf[left];
        left = (left + 1) % full_len;
    }

    count -= nwrite;
    if (len > nwrite)
        for (int i = nwrite; i < len; ++i) {
            stream[i] = 0;
        }
}


void init_sdl_audio() {

    count = 0;
    desired.format = AUDIO_S16SYS;
    desired.userdata = NULL;
    desired.callback = audio_play;
    int ret = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if (ret == 0) {
        SDL_OpenAudio(&desired, NULL);
        SDL_PauseAudio(0);

    } else
        printf("%s\n", SDL_GetError());
}


static void audio_io_handler(uint32_t offset, int len, bool is_write) {
    if (is_write) {
        switch (offset) {
            case reg_count * 4: {
                count = audio_base[reg_count];
            }
            case reg_freq * 4: {
                desired.freq = audio_base[reg_freq];
                break;
            }
            case reg_channels * 4: {
                desired.channels = audio_base[reg_channels];
                break;
            }
            case reg_samples * 4: {
                desired.samples = audio_base[reg_samples];
                init_sdl_audio();
                break;
            }
            case reg_init * 4: {
                right = audio_base[reg_init];
                break;
            }
        }
    } else {
        switch (offset) {
            case reg_init * 4: {
                audio_base[reg_init] = right;
                break;
            }
            case reg_count * 4: {
                audio_base[reg_count] = count;
                break;
            }
            case reg_sbuf_size * 4: {
                audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
            }
        }
    }
}

void init_audio() {
    uint32_t space_size = sizeof(uint32_t) * nr_reg;
    audio_base = (uint32_t *) new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
    add_pio_map("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
    add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

    sbuf = (uint8_t *) new_space(CONFIG_SB_SIZE);
    left = 0;
    right = 0;
    full_len = CONFIG_SB_SIZE / sizeof(uint8_t);
    add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}

#include <am.h>
#include <nemu.h>
#include <stdio.h>
#define AUDIO_FREQ_ADDR (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR (AUDIO_ADDR + 0x14)


void __am_audio_init() {
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
    cfg->present = true;
    cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
    outl(AUDIO_FREQ_ADDR, ctrl->freq);
    outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
    outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
    stat->count = inl(AUDIO_COUNT_ADDR);
}

void audio_write(uint8_t *sta, int len) {
    printf("1111\n");
    int buf_size = inl(AUDIO_SBUF_SIZE_ADDR);
    int full_len = buf_size / sizeof(uint8_t);
    printf("FFF::%d\n",full_len);
    int count = inl(AUDIO_COUNT_ADDR);
    outl(AUDIO_COUNT_ADDR, count + len);
    uint8_t *ptr = (uint8_t *) (uintptr_t) AUDIO_SBUF_ADDR;
    int right = inl(AUDIO_INIT_ADDR);
    for (int i = 0; i < len; ++i) {
        ptr[right] = sta[i];
        right = (right + 1) % full_len;
    }
    outl(AUDIO_INIT_ADDR, right);
    printf("2222\n");
}


void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
    uint8_t *sta = (uint8_t *) ctl->buf.start;
    uint8_t *end = (uint8_t *) ctl->buf.end;
    int len = end - sta;
    audio_write(sta, len);
}

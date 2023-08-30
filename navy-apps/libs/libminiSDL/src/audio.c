#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <stdlib.h>

static SDL_AudioSpec *detail = NULL;
static int invoke_time = 0;
static int last_invoke_time = 0;
static bool flag = false;
extern uint32_t SDL_GetTicks();
extern void NDL_OpenAudio(int freq, int channels, int samples);
extern int NDL_PlayAudio(void *buf, int len);
extern void NDL_CloseAudio();

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
    detail = desired;
    assert(detail);
    flag = false;
    invoke_time = 1000 / detail->freq;
    printf(" TEST :: %d\n", invoke_time);
    assert(invoke_time > 1);
    NDL_OpenAudio(detail->freq, detail->channels, detail->samples);
    return 0;
}

void SDL_CloseAudio() {
    NDL_CloseAudio();
}

void SDL_PauseAudio(int pause_on) {
    if (pause_on == 0) {//开始播放
        flag = true;
        last_invoke_time = SDL_GetTicks();
    } else {
        flag = false;
    }
}


void CallbackHelper() {
    if (flag) {
        assert(detail);
        if (SDL_GetTicks() - last_invoke_time >= invoke_time) {
            last_invoke_time = SDL_GetTicks();
            void *ptr = malloc(detail->samples);
            detail->callback(detail->userdata, ptr, detail->samples);
            assert(ptr);
            NDL_PlayAudio(ptr, detail->samples);
        }
    }
}


void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
    assert(0);
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
    assert(0);
    return NULL;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
    assert(0);
}

void SDL_LockAudio() {
    assert(0);
}

void SDL_UnlockAudio() {
    assert(0);
}

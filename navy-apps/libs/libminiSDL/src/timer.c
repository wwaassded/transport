#include <NDL.h>
#include <assert.h>
#include <sdl-timer.h>
#include <stdio.h>

extern uint32_t NDL_GetTicks();

SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback, void *param) {
    assert(0);
    return NULL;
}

int SDL_RemoveTimer(SDL_TimerID id) {
    assert(0);
    return 1;
}

uint32_t SDL_GetTicks() {
    return NDL_GetTicks();
}

void SDL_Delay(uint32_t ms) {
    assert(0);
}

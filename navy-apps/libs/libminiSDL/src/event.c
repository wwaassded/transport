#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#define keyname(k) #k,

static const char *keyname[] = {
        "NONE",
        _KEYS(keyname)};

int SDL_PushEvent(SDL_Event *ev) {
    assert(0);
    return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
    assert(0);
    return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
    // TODO :: implament me !!
    int fd = open("/dev/events", 0, 0);
    char buf[32];
    int ret = read(fd, buf, 0);
    if (ret != 0) {
        char *typ = strtok(buf, " ");
        typ = strtok(NULL, " ");
        typ = strtok(NULL, " ");
        typ = strtok(NULL, " ");
        if (strcmp(typ, "kd") == 0) {
            event->type = SDL_KEYDOWN;
            event->key.keysym.sym = ret;
        } else if (strcmp(typ, "ku") == 0)
            event->type = SDL_KEYUP;
        else
            assert(0);
    }
    return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
    assert(0);
    return 0;
}

uint8_t *SDL_GetKeyState(int *numkeys) {
    assert(0);
    return NULL;
}

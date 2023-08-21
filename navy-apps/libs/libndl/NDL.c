#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;

uint32_t NDL_GetTicks() {
    return gettimeofday(NULL, NULL) * 1000;
    return 0;
}

int NDL_PollEvent(char *buf, int len) {
    int fd = open("/dev/events", 0, 0);
    return read(fd, buf, len);
}

void NDL_OpenCanvas(int *w, int *h) {
    if (getenv("NWM_APP")) {
        int fbctl = 4;
        fbdev = 5;
        screen_w = *w;
        screen_h = *h;
        char buf[64];
        int len = sprintf(buf, "%d %d", screen_w, screen_h);
        printf("%s\n", buf);
        // let NWM resize the window and create the frame buffer
        write(fbctl, buf, len);
        while (1) {
            // 3 = evtdev
            int nread = read(3, buf, sizeof(buf) - 1);
            if (nread <= 0) continue;
            buf[nread] = '\0';
            if (strcmp(buf, "mmap ok") == 0) break;
        }
        close(fbctl);
    } else {
        char buf[64];
        int fd = open("/proc/dispinfo", 0, 0);
        read(4, buf, 64);
        char *tmp = strtok(buf, " ");
        char *number = strtok(NULL, " ");
        number = strtok(NULL, " ");
        if (strcmp(tmp, "WIDTH") == 0)
            screen_w = atoi(number);
        else if (strcmp(tmp, "HEIGHT") == 0)
            screen_h = atoi(number);
        else
            assert(0);
        tmp = strtok(NULL, " ");
        number = strtok(NULL, " ");
        number = strtok(NULL, " ");
        if (strcmp(tmp, "WIDTH") == 0)
            screen_w = atoi(number);
        else if (strcmp(tmp, "HEIGHT") == 0)
            screen_h = atoi(number);
        else
            assert(0);
        if (*w == 0 && *h == 0) {
            *w = screen_w;
            *h = screen_h;
        }
        assert(screen_h >= *h && screen_w >= *w);
    }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
    int fd = open("/dev/fb", 0, 0);
    for (int i = y; i < y + h; ++i) {
        lseek(fd, (i * screen_w + x) * sizeof(uint32_t), SEEK_SET);
        int erro = write(fd, pixels, w * sizeof(uint32_t));
        assert(erro != -1);
    }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
    return 0;
}

int NDL_QueryAudio() {
    return 0;
}

int NDL_Init(uint32_t flags) {
    if (getenv("NWM_APP")) {
        evtdev = 3;
    }
    return 0;
}

void NDL_Quit() {
}

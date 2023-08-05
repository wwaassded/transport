#include <am.h>
#include <nemu.h>
#include <stdio.h>
#define SYNC_ADDR (VGACTL_ADDR + 4)
static int ww = 0;
static int hh = 0;
void __am_gpu_init() {
    int i;
    uint32_t screen_config = inl(VGACTL_ADDR + 0);
    int w = screen_config >> 16;
    int h = screen_config & (0x0000ffff);
    ww = w;
    hh = h;
    uint32_t *fb = (uint32_t *) (uintptr_t) FB_ADDR;
    for (i = 0; i < w * h; i++) fb[i] = i;
    outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
    uint32_t screen_config = inl(VGACTL_ADDR + 0);
    *cfg = (AM_GPU_CONFIG_T){
            .present = true,
            .has_accel = false,
            .width = screen_config >> 16,
            .height = screen_config & (0x0000ffff),
            .vmemsz = 0};
}
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
    if (ctl->sync) {
        outl(SYNC_ADDR, 1);
    } else {
        int x = ctl->x;
        int y = ctl->y;
        uint32_t *fb = (uint32_t *) (uintptr_t) FB_ADDR;
        int h = ctl->h;
        int w = ctl->w;
        uint32_t *canava = (uint32_t *) ctl->pixels;
        int i = 0, j = 0;
        for (i = 0; i < h; ++i)
            for (j = 0; j < w; ++j) {
                if (y + i < hh && x + j < ww)
                    fb[(y + i) * hh + (x + j)] = canava[i * w + j];
            }
        outl(SYNC_ADDR, 0);
    }
}
void __am_gpu_status(AM_GPU_STATUS_T *status) {
    status->ready = true;
}

#include <am.h>
#include <nemu.h>
#include <stdlib.h>
#include <string.h>
#define SYNC_ADDR (VGACTL_ADDR + 4)
static int W = 0;
static int H = 0;
uint32_t min(uint32_t a, uint32_t b) {
    if (a < b)
        return a;
    else
        return b;
}
void __am_gpu_init() {
    int i;
    uint32_t screen_config = inl(VGACTL_ADDR + 0);
    W = screen_config >> 16;
    H = screen_config & (0x0000ffff);
    uint32_t *fb = (uint32_t *) (uintptr_t) FB_ADDR;
    for (i = 0; i < W * H; i++) fb[i] = i;
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
    if (ctl->sync)
        outl(SYNC_ADDR, 1);
    int x = ctl->x;
    int y = ctl->y;
    uint32_t *fb = (uint32_t *) (uintptr_t) FB_ADDR;
    int h = ctl->h;
    int w = ctl->w;
    uint32_t *pixels = (uint32_t *) ctl->pixels;
    int cp_bytes = sizeof(uint32_t) * min(w, W - x);
    for (int j = 0; j < h && y + j < H; ++j) {
        memcpy(&fb[(y + j) * W + x], pixels, cp_bytes);
        pixels += w;
    }
}
void __am_gpu_status(AM_GPU_STATUS_T *status) {
    status->ready = true;
}

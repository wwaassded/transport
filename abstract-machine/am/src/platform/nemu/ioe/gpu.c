#include <am.h>
#include <nemu.h>
#include <stdio.h>
#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
    int i;
    uint32_t screen_config = inl(VGACTL_ADDR + 0);
    int w = screen_config >> 16;
    int h = screen_config & (0x0000ffff);
    uint32_t *fb = (uint32_t *) (uintptr_t) FB_ADDR;
    for (i = 0; i < w * h; i++) fb[i] = i;
    outl(SYNC_ADDR, 0x00ffffff);
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
// CONFIG_FB_ADDR
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
    // int ii = 0;
    // void *ptr = (void *) 0xa1000000;
    // for (int i = 0; i < ctl->w; ++i)
    //     for (int j = 0; j < ctl->h; ++j) {
    //         outl(((uint32_t *) ptr)[ctl->y + j + (ctl->x + i) * ctl->h], *(uint32_t *) (ctl->pixels + ii));
    //         ++ii;
    //     }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
    status->ready = true;
}

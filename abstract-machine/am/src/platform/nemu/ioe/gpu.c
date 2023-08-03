#include <am.h>
#include <nemu.h>
#include <stdio.h>
#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
    int i;
    int w = 0;// TODO: get the correct width
    int h = 0;// TODO: get the correct height
    uint32_t *fb = (uint32_t *) (uintptr_t) FB_ADDR;
    for (i = 0; i < w * h; i++) fb[i] = i;
    outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
    uint32_t screen_config = inl(VGACTL_ADDR + 0);
    printf("ALL:0x%08x\n",screen_config);
    *cfg = (AM_GPU_CONFIG_T){
            .present = true,
            .has_accel = false,
            .width = screen_config >> 16,
            .height = screen_config & (0x0000ffff),
            .vmemsz = 0};
    if (cfg->height != (screen_config - (screen_config >> 16))) {
        printf("FAG:0x%08x  0x%08x\n",cfg->width,cfg->height);
    }
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
    status->ready = true;
}

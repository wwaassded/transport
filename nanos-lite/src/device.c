#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
#define MULTIPROGRAM_YIELD() yield()
#else
#define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
    [AM_KEY_##key] = #key,


static const char *keyname[256] __attribute__((used)) = {
        [AM_KEY_NONE] = "NONE",
        AM_KEYS(NAME)};
extern void _putch(char ch);
static int screen_w = 0, screen_h = 0;

size_t serial_write(const void *buf, size_t offset, size_t len) {
    char *buf_char = (char *) buf;
    for (size_t i = 0; i < len; ++i)
        _putch(buf_char[i]);
    return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
    AM_INPUT_KEYBRD_T kev = io_read(AM_INPUT_KEYBRD);
    if (kev.keycode == AM_KEY_NONE)
        return 0;
    sprintf(buf, "receive event : %s %s\n", kev.keydown ? "kd" : "ku", keyname[kev.keycode]);
    return 1;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
    AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
    sprintf(buf, "WIDTH : %d HEIGHT : %d", cfg.width, cfg.height);
    return 1;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
    int w = len / sizeof(uint32_t);
    int number = offset / sizeof(uint32_t);
    void *pixels = (void *) buf;
    io_write(AM_GPU_FBDRAW, number % screen_w, number / screen_w, pixels, w, 1, true);
    printf("TT:%d\n", number / screen_w);
    return len;
}

size_t init_fb() {
    AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
    screen_w = cfg.width;
    screen_h = cfg.height;
    return cfg.height * cfg.width * sizeof(uint32_t);
}

void init_device() {
    Log("Initializing devices...");
    ioe_init();
}

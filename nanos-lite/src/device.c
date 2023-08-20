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
    sprintf(buf, "WIDTH : %u\nHEIGHT : %u\n", cfg.width, cfg.height);
    return 1;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
    return 0;
}

void init_device() {
    Log("Initializing devices...");
    ioe_init();
}

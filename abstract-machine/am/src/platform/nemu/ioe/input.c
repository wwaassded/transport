#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000
// KBD_ADDR
void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
    uint32_t tmp = *(uint32_t *) (KBD_ADDR + 0);
    if ((tmp & KEYDOWN_MASK) == 0x8000) {
        kbd->keydown = true;
        kbd->keycode = tmp - KEYDOWN_MASK;
    } else {
        kbd->keydown = false;
        kbd->keycode = tmp;
    }
}

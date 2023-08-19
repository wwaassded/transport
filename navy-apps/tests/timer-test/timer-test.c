#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

extern uint32_t NDL_GetTicks();

int main() {
    int sec = 0;
    while (1) {
        while (NDL_GetTicks() / 1000 < sec)
            ;
        printf("i am a test\n");
        ++sec;
    }
    return 0;
}
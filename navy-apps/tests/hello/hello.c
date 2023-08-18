#include <assert.h>
#include <stdio.h>
#include <unistd.h>
extern intptr_t _syscall_(intptr_t type, intptr_t a0, intptr_t a1, intptr_t a2);
int main() {
    write(1, "Hello World!\n", 13);
    printf("only test!\n");
    printf("FUCK\n");
    _syscall_(0, 0, 0, 0);
    // assert(0);
    // int i = 2;
    // volatile int j = 0;
    // while (1) {
    //     j++;
    //     if (j == 10000) {
    //         printf("Hello World from Navy-apps for the %dth time!\n", i++);
    //         j = 0;
    //     }
    // }
}

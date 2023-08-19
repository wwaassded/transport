#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


int main() {
    int sec = 0;
    while (1) {
        while (gettimeofday(NULL, NULL) < sec)
            ;
        printf("i am a test\n");
        ++sec;
    }
    return 0;
}
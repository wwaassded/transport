#include <stdio.h>
#include <unistd.h>

int main() {
    write(1, "Hello World!\n", 13);
    int number = 2;
    for (; number < 100; ++number)
        printf("FUCK:%d\n", number);
    return 0;
}

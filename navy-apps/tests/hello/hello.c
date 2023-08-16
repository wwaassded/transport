#include <stdio.h>
#include <unistd.h>

int main() {
    write(1, "Hello World!\n", 13);
    int number = 2;
    for (int i = number; i < 100; ++i)
        printf("FUCK:%d\n", i);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


int main() {
    int res = gettimeofday(NULL, NULL);
    printf("%d\n", res);
    return 0;
}
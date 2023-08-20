#include <BMP.h>
#include <NDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
extern void *_sbrk(__intptr_t increment);
extern char _end;
int main() {
    NDL_Init(0);
    int w, h;
    void *bmp = BMP_Load("/share/pictures/projectn.bmp", &w, &h);
    printf("ori:%d %d\n", w, h);
    assert(bmp);
    NDL_OpenCanvas(&w, &h);
    printf("pre:%d %d\n", w, h);
    assert(0);
    NDL_DrawRect(bmp, 0, 0, w, h);
    free(bmp);
    NDL_Quit();
    printf("%d %d\n", w, h);
    printf("Test ends! Spinning...\n");
    while (1)
        ;
    return 0;
}

#include <unistd.h>
#include <stdio.h>

int main() {
  write(1, "Hello World!\n", 13);
  int number = 2;
  printf("FUCK:%d\n",number);
  return 0;
}

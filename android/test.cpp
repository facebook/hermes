#include <stdint.h>
#include <stdio.h>

volatile double D = 0xFFFFFFFF;

int convertWithSSE2(double n) {
  int result;
  asm("cvttsd2si %1, %0" : "=r"(result) : "x"(n) :);
  return result;
}

int convertWithFPU(double n) {
  int result;
  asm("fldl %1\n\t"
      "fisttpl %0"
      : "=m"(result)
      : "m"(n)
      : "st");
  return result;
}

int main(void) {
  volatile int x = (int)D;
  printf("D: %f, int: %x\n", D, x);
  volatile unsigned ux = (unsigned)D;
  printf("D: %f, unsigned: %x\n", D, ux);
  int u1 = convertWithSSE2(D);
  printf("cvttsd2si: %x\n", u1);
  int u2 = convertWithFPU(D);
  printf("fisttp: %x\n", u2);
  return 0;
}

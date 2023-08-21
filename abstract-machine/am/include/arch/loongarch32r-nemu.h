#ifndef __ARCH_H__
#define __ARCH_H__

struct Context {
  uintptr_t gpr[32], era, estat, prmd;
  void *pdir;
};

#define GPR1 gpr[11] // a7
#define GPR2 gpr[0]
#define GPR3 gpr[0]
#define GPR4 gpr[0]
#define GPRx gpr[0]

#endif

#include <elf.h>
#include <proc.h>
#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif
extern size_t get_ramdisk_size();
static uintptr_t loader(PCB *pcb, const char *filename) {
    printf("FFF::%d\n", get_ramdisk_size());
    panic("HERE!");
    return 0;
}

void naive_uload(PCB *pcb, const char *filename) {
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = %p", entry);
    ((void (*)()) entry)();
}

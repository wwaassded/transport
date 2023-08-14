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
extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);
extern void init_ramdisk();


static uintptr_t loader(PCB *pcb, const char *filename) {
    Elf_Ehdr ELF_head;
    ramdisk_read(&ELF_head, 0, sizeof(Elf_Ehdr));
    unsigned char magic_number[] = {0x7f, 'E', 'L', 'F'};
    if (memcpy(ELF_head.e_ident, magic_number, 4) != 0)
        panic("it is not a elf file!");
    panic("YEE!");
}

void naive_uload(PCB *pcb, const char *filename) {
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = %p", entry);
    ((void (*)()) entry)();
}

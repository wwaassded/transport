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
// unsigned char expected_magicnumber[] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};
// if (memcmp(elf_hdr.e_ident, expected_magicnumber, sizeof(expected_magicnumber)) != 0) {
//     printf("%s is not a elf file please check your makefile!\n", elf_file);
//     return;
// }

static uintptr_t loader(PCB *pcb, const char *filename) {
    Elf_Ehdr ELF_head;
    ramdisk_read(&ELF_head, 0, sizeof(Elf_Ehdr));
    unsigned char magic_number[] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};
    if (memcmp(ELF_head.e_ident, magic_number, sizeof(magic_number)) != 0)
        panic("it is not a elf file! please check resources.S or set correct ramdisk.img!");
    assert(ELF_head.e_ident[4] == ELFCLASS32);
    panic("YEE!");
    // uint32_t entry_address = ELF_head.e_entry;
    Elf32_Phdr ELF_phead;
    for (int i = 0; i < ELF_head.e_phnum; ++i) {
        size_t offset = ELF_head.e_phoff + i * ELF_head.e_phentsize;
        ramdisk_read(&ELF_phead, offset, sizeof(Elf32_Phdr));
        switch (ELF_phead.p_type) {
            case PT_LOAD: {

                break;
            }
            default: {
            }
        }
    }
}

void naive_uload(PCB *pcb, const char *filename) {
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = %p", entry);
    ((void (*)()) entry)();
}

#include <elf.h>
#include <proc.h>
#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif

typedef int file_dp;

extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_read(file_dp fd, void *buf, size_t len);
extern size_t fs_lseek(file_dp fd, size_t offset, int whence);
extern int fs_close(file_dp fd);


static uintptr_t loader(PCB *pcb, const char *filename) {
    file_dp fd = fs_open(filename, 0, 0);
    Elf_Ehdr ELF_head;
    fs_lseek(fd, 0, 0);
    fs_read(fd, &ELF_head, sizeof(Elf_Ehdr));
    unsigned char magic_number[] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};
    if (memcmp(ELF_head.e_ident, magic_number, sizeof(magic_number)) != 0)
        panic("it is not a elf file! please check resources.S or set correct ramdisk.img!");
    assert(ELF_head.e_ident[4] == ELFCLASS32);
    Elf32_Phdr ELF_phead;
    for (int i = 0; i < ELF_head.e_phnum; ++i) {
        size_t offset = ELF_head.e_phoff + i * ELF_head.e_phentsize;
        fs_lseek(fd, offset, 0);
        fs_read(fd, &ELF_phead, sizeof(Elf32_Phdr));
        switch (ELF_phead.p_type) {
            case PT_LOAD: {
                uint8_t *mem_ptr = (uint8_t *) (uintptr_t) ELF_phead.p_vaddr;
                memset(mem_ptr, 0, ELF_phead.p_memsz);
                fs_lseek(fd, ELF_phead.p_offset, 0);
                fs_read(fd, mem_ptr, ELF_phead.p_filesz);
                break;
            }
            default: {
            }
        }
    }
    fs_close(fd);
    return (uintptr_t) ELF_head.e_entry;
}


void naive_uload(PCB *pcb, const char *filename) {
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = %u", entry);
    ((void (*)()) entry)();
}

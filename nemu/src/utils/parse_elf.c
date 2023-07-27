#include <common.h>
#include <elf.h>
#include <sys/mman.h>

static FILE *fp;
static void *elf = NULL;

void parse_elf(const char *elf_file);

void init_elf(const char *elf_file) {
    assert(elf_file);
    fp = fopen(elf_file, "r");
    if (fp == NULL)
        printf("can not open your elf_file!\n");
    else {
        fseek(fp, 0, SEEK_END);
        size_t number = ftell(fp);
        elf = mmap(NULL, number, PROT_READ, MAP_PRIVATE, fileno(fp), 0);
        fclose(fp);
        if (elf == MAP_FAILED)
            perror("in_init_elf");
        else
            parse_elf(elf_file);
    }
}

void parse_elf(const char *elf_file) {
    assert(elf);
    Elf32_Ehdr elf_hdr;
    memmove(&elf_hdr, elf, sizeof(Elf32_Ehdr));
    unsigned char expected_magicnumber[] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};
    if (memcmp(elf_hdr.e_ident, expected_magicnumber, sizeof(expected_magicnumber)) != 0) {
        printf("%s is not a elf file please check your makefile!\n", elf_file);
        return;
    }
    if (elf_hdr.e_ident[EI_CLASS] != ELFCLASS32) {
        printf("only 32bit elf file is supported!\n");
        return;
    }
    size_t offset_sym = 0;
    size_t offset_str = 0;
    size_t size_sym = 0;
    uint16_t i;
    for (i = 0; i < elf_hdr.e_shnum; ++i) {
        size_t offset = elf_hdr.e_shoff + i * elf_hdr.e_shentsize;
        Elf32_Shdr shdr;
        memmove(&shdr, elf + offset, sizeof(Elf32_Shdr));
        switch (shdr.sh_type) {
            case SHT_SYMTAB: {
                size_sym = shdr.sh_size;
                offset_sym = shdr.sh_offset;
                printf("SYMTAB:%d\n", shdr.sh_offset);
                break;
            }
            case SHT_STRTAB: {
                if (offset_str == 0)
                    offset_str = shdr.sh_offset;
                printf("STRTAB:%d\n", shdr.sh_offset);
            }
            default:
                break;
        }
    }
    assert(offset_sym != 0 && offset_str != 0 && size_sym);

}

void func() {
    printf("Hello World\n!");
}
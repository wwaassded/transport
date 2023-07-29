#include <common.h>
#include <cpu/decode.h>
#include <elf.h>
#include <isa.h>
#include <sys/mman.h>

#define FUNC_NUMBER 128

static FILE *fp;
static void *elf = NULL;
static Func_Info func_info[FUNC_NUMBER];
static uint32_t F_len = 0;
static FILE *ftrace_fp = NULL;
static uint32_t number = 0;
static struct {
    uint32_t sym_size;
    uint32_t sym_offset;
    uint32_t str_offset;
} elf_info;

void init_Func_Info() {
    char *cbytes = (char *) elf;
    uint32_t j = 0;
    for (j = 0; j * sizeof(Elf32_Sym) < elf_info.sym_size; ++j) {
        Elf32_Sym tmp;
        uint32_t absoffset = elf_info.sym_offset + j * sizeof(Elf32_Sym);
        memmove(&tmp, cbytes + absoffset, sizeof(Elf32_Sym));
        if (tmp.st_name != 0 && ELF32_ST_TYPE(tmp.st_info) == STT_FUNC && tmp.st_size != 0) {
            strncpy(func_info[F_len].F_name, elf_info.str_offset + cbytes + tmp.st_name, FUNC_NAME_LEN);
            func_info[F_len].sta_address = tmp.st_value;
            func_info[F_len].size = tmp.st_size;
            ++F_len;
        }
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
                break;
            }
            case SHT_STRTAB:
                if (offset_str == 0)
                    offset_str = shdr.sh_offset;
            default:
                break;
        }
    }
    assert(offset_sym != 0 && offset_str != 0 && size_sym);
    elf_info.str_offset = offset_str;
    elf_info.sym_offset = offset_sym;
    elf_info.sym_size = size_sym;
    init_Func_Info();
}

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
    assert(!ftrace_fp);
    ftrace_fp = fopen("/root/operater_system/nemu/src/cpu/ftrace.txt", "w");
    if (ftrace_fp == NULL)
        perror("fail to open file");
}

void parse_decode(Decode *s, vaddr_t pc) {
    assert(ftrace_fp);
    if (strncmp(s->name, "jal", CMD_LEN) == 0 || strncmp(s->name, "jalr", CMD_LEN) == 0) {
        uint32_t ii = 0;
        uint16_t ori = 0;
        int16_t tar = 0;
        uint16_t sta = F_len + 1;
        uint16_t end = sta;
        for (ii = 0; ii < F_len; ++ii) {
            sta = func_info[ii].sta_address;
            end = sta + func_info[ii].size;
            if (s->dnpc == sta) {
                tar = -ii;
                break;
            }
            if (pc >= sta && pc < end) {
                ori = ii;
                break;
            }
            if (s->dnpc > sta && s->dnpc < end) {
                tar = ii;
                break;
            }
        }
        printf("%08x %08x\n",pc,s->dnpc);
        assert(ori != F_len + 1);
        if (tar != F_len + 1 && strcmp(func_info[abs(tar)].F_name, func_info[ori].F_name) != 0) {
            if (tar < 0) {
                printf("FUCK!\n");
                tar = -tar;
                for (ii = 0; ii < number; ++ii)
                    fprintf(ftrace_fp, " ");
                fprintf(ftrace_fp, "[0x%08x:call %s in %s]\n", pc, func_info[tar].F_name, func_info[ori].F_name);
                ++number;
            } else if (strcmp(s->name, "jalr")) {
                --number;
                for (ii = 0; ii < number; ++ii)
                    fprintf(ftrace_fp, " ");
                fprintf(ftrace_fp, "[0x%08x:ret %s in %s]\n", pc, func_info[tar].F_name, func_info[ori].F_name);
            }
        }
    }
}
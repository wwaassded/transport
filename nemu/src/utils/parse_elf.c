#include <common.h>
#include <elf.h>
#include <sys/mman.h>

static FILE *fp;
static void *elf = NULL;

void parse_elf(const char *elf_file);

void init_elf(const char *elf_file)
{
  assert(elf_file);
  fp = fopen(elf_file, "r");
  if (fp == NULL)
    printf("can not open your elf_file!\n");
  else
  {
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


void parse_elf(const char *elf_file)
{
  assert(elf);
  Elf32_Ehdr elf_hdr;
  memmove(&elf_hdr,elf,sizeof(Elf32_Ehdr));
  unsigned char expected_magicnumber[] = {ELFMAG0,ELFMAG1,ELFMAG2,ELFMAG3};
  printf("\n\nSTROFF::0x%08x\n\n",elf_hdr.e_shstrndx);
  if(memcmp(elf_hdr.e_ident,expected_magicnumber,sizeof(expected_magicnumber)) != 0) {
    printf("%s is not a elf file please check your makefile!\n",elf_file);
    return;
  }
  if(elf_hdr.e_ident[EI_CLASS] != ELFCLASS32) {
    printf("only 32bit elf file is supported!\n");
    return;
  }
  printf("\n\nSTROFF::0x%08x\n\n",elf_hdr.e_shstrndx);
}
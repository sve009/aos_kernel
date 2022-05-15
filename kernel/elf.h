#pragma once

#define EI_NIDENT 16

typedef struct {
  unsigned char e_ident[EI_NIDENT];
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  uint64_t e_entry;
  uint64_t e_phoff;
  uint64_t e_shoff;
  uint32_t e_flags;
  uint16_t e_ehsize;
  uint16_t e_phentsize;
  uint16_t e_phnum;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrndx;
} __attribute__((packed)) Elf64_Ehdr;

// Note for type:
// 2 = symbol table
// 3 = string table
typedef struct {
	uint64_t sh_name;
	uint64_t sh_type;
	uint64_t sh_flags;
	uint64_t sh_addr;
	uint64_t sh_offset;
	uint64_t sh_size;
	uint64_t sh_link;
	uint64_t sh_info;
	uint64_t sh_addralign;
	uint64_t sh_entsize;
} __attribute__((packed)) Elf64_Shdr;

typedef struct {
	uint64_t	st_name;
	unsigned char	st_info;
	unsigned char	st_other;
	uint32_t	st_shndx;
	uint64_t	st_value;
	uint64_t	st_size;
} __attribute__((packed)) Elf64_Sym;

typedef struct {
  uint32_t p_type;
  uint32_t p_flags;
  uint64_t p_offset;
  uint64_t p_vaddr;
  uint64_t p_paddr;
  uint64_t p_filesz;
  uint64_t p_memsz;
  uint64_t p_align;
} __attribute__((packed)) Elf64_Phdr;


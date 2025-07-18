/**
 * ELF相关头文件及配置
 */
#ifndef OS_ELF_H
#define OS_ELF_H

#include "types.h"

// ELF相关数据类型
typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

#pragma pack(1)

// ELF Header
#define EI_NIDENT       16
#define ELF_MAGIC       0x7F   // elf文件第一个字节，为固定格式

#define ET_EXEC         2   // 可执行文件
#define ET_386          3   // 80386处理器

#define PT_LOAD         1   // 可加载类型
typedef struct {
    char e_ident[EI_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;          // program header 偏移
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;         // program header 数量
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
}Elf32_Ehdr;

#define PT_LOAD         1

typedef struct {
    Elf32_Word p_type;
    Elf32_Off p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
} Elf32_Phdr;                  // 存储段信息

#pragma pack()

#endif //OS_ELF_H

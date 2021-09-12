/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _ELF_H
#define _ELF_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define ELFMAG      "\177ELF"
#define SELFMAG     4

#define EI_NIDENT   16

typedef struct _elf64_hdr {
    unsigned char e_ident[EI_NIDENT]; /* ELF "magic number" */
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;          /* Entry point virtual address */
    uint64_t e_phoff;          /* Program header table file offset */
    uint64_t e_shoff;          /* Section header table file offset */
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} elf64_hdr;

static inline bool
elf64_hdr_check(void *ptr)
{
    return (!memcmp(ptr, ELFMAG, SELFMAG));
}

static inline void
elf64_hdr_set_length(void *ptr, uint64_t length)
{
    elf64_hdr *hdr = (elf64_hdr *)ptr;
    /* use e_phoff as file length since it's useless fo *.ko */
    hdr->e_phoff = length;
}

#endif /* _ELF_H */

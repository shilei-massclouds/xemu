/*
 * Module
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "module.h"
#include "util.h"

#define PAGE_OFFSET 0xffffffe000000000UL
#define KMODULE_DIR "image/"

LIST_HEAD(pending);
LIST_HEAD(modules);

LIST_HEAD(symbols);

uint32_t ksym_ptr;
uint32_t ksym_num;

static bool
check_module(const char *name)
{
    char *p = strrchr(name, '.');
    return !strcmp(p, ".ko");
}

static void
discover_modules(void)
{
    int n;
    struct dirent **namelist;
    bool has_startup = false;
    bool has_system_map = false;

    n = scandir(KMODULE_DIR, &namelist, NULL, NULL);
    if (n == -1) {
        panic("%s: error when scandir\n");
    }

    while (n--) {
        if (check_module(namelist[n]->d_name)) {
            module *mod = malloc(sizeof(module));
            mod->name = strdup(namelist[n]->d_name);
            INIT_LIST_HEAD(&(mod->undef_syms));
            list_add_tail(&(mod->list), &pending);
        } else if (!strcmp(namelist[n]->d_name, "startup.bin")) {
            has_startup = true;
        } else if (!strcmp(namelist[n]->d_name, "System.map")) {
            has_system_map = true;
        }

        free(namelist[n]);
    }
    free(namelist);

    if (!has_startup)
        panic("%s: No startup.bin\n", __func__);

    if (!has_system_map)
        panic("%s: No System.map\n", __func__);
}

static void
detect_syms_range(void)
{
    FILE *fp;
    char line[256];
    uint64_t start = 0;
    uint64_t end = 0;

    fp = fopen(KMODULE_DIR "System.map", "r");
    if (fp == NULL)
        panic("No System.map\n");

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, "_start_ksymtab_strings")) {
            char *tail = strchr(line, ' ');
            start = strtoul(line, &tail, 16);
        } else if (strstr(line, "_end_ksymtab_strings")) {
            char *tail = strchr(line, ' ');
            end = strtoul(line, &tail, 16);
        }

        if (start && end)
            break;
    }

    fclose(fp);

    ksym_ptr = (uint32_t)(start - PAGE_OFFSET);
    ksym_num = (uint32_t)(end - start);
}

static void
export_symbols(const char *start, const char *end)
{
    while (start < end) {
        symbol *sym = malloc(sizeof(symbol));
        sym->name = strdup(start);
        list_add_tail(&(sym->list), &symbols);

        printf("%s: export(%s)\n", __func__, start);
        start = strchr(start, '\0');
        start++;
    }
}

static void
init_symtable(void)
{
    FILE *fp;
    char *buf;
    char *cur;

    detect_syms_range();
    printf("range(%x, %x)\n", ksym_ptr, ksym_num);

    fp = fopen(KMODULE_DIR "startup.bin", "rb");
    if (fp == NULL)
        panic("No System.map\n");

    fseek(fp, (long)ksym_ptr, SEEK_SET);

    buf = malloc(ksym_num);
    fread(buf, 1, ksym_num, fp);

    export_symbols(buf, buf + ksym_num);

    free(buf);
    fclose(fp);
}

static char *
get_strtab(Elf64_Shdr *shdr, FILE *fp)
{
    char *str;

    str = (char *) malloc(shdr->sh_size);
    printf("%s: offset(%lx)\n", __func__, shdr->sh_offset);

    fseek(fp, (long)shdr->sh_offset, SEEK_SET);
    if (fread(str, 1, shdr->sh_size, fp) != shdr->sh_size)
        panic("Read str section error\n");

    return str;
}

static void
discover_undef_syms(module *mod,
                    Elf64_Shdr *shdr,
                    const char *strtab,
                    FILE *fp)
{
    int i;
    Elf64_Sym *sym;

    sym = (Elf64_Sym *)malloc(shdr->sh_size);

    fseek(fp, (long)shdr->sh_offset, SEEK_SET);
    if (fread(sym, 1, shdr->sh_size, fp) != shdr->sh_size)
        panic("Read sym section error\n");

    for (i = 1; i < shdr->sh_size / sizeof(Elf64_Sym); i++) {
        if (sym[i].st_shndx == SHN_UNDEF) {
            symbol *undef = malloc(sizeof(symbol));
            undef->name = strdup(strtab + sym[i].st_name);
            list_add_tail(&(undef->list), &(mod->undef_syms));
            printf("%s: name(%s)\n", __func__, undef->name);
        }
    }

    free(sym);
}

static void
analysis_module(module *mod)
{
    int i;
    FILE *fp;
    Elf64_Shdr *sechdrs;
    char *secstrings;
    Elf64_Ehdr hdr = {0};
    char filename[256] = {0};

    printf("%s: %s\n", __func__, mod->name);
    sprintf(filename, "%s%s", KMODULE_DIR, mod->name);
    fp = fopen(filename, "rb");
    if (fp == NULL)
        panic("bad file (%s)\n", filename);

    if (fread(&hdr, sizeof(Elf64_Ehdr), 1, fp) != 1)
        panic("Read elf header error\n");

    fseek(fp, (long)hdr.e_shoff, SEEK_SET);

    sechdrs = calloc(hdr.e_shnum, sizeof(Elf64_Shdr));

    if (fread(sechdrs, sizeof(Elf64_Shdr), hdr.e_shnum, fp) != hdr.e_shnum)
        panic("Read section header error\n");

    secstrings = get_strtab(&sechdrs[hdr.e_shstrndx], fp);

    for (i = 0; i < hdr.e_shnum; i++) {
        Elf64_Shdr *shdr = sechdrs + i;

        if (shdr->sh_type == SHT_SYMTAB) {
            char *strtab;
            strtab = get_strtab(&sechdrs[shdr->sh_link], fp);

            printf("[%d]: SHT_SYMTAB (%lx, %lx)\n",
                   i, shdr->sh_offset, shdr->sh_size);

            discover_undef_syms(mod, shdr, strtab, fp);
            free(strtab);
        } else {
            const char *name = secstrings + shdr->sh_name;
            if (strcmp(name, "_ksymtab_strings") == 0) {
                char *ksym_str = get_strtab(shdr, fp);
                export_symbols(ksym_str, ksym_str + shdr->sh_size);
            }
        }
    }

    free(secstrings);
    free(sechdrs);
    fclose(fp);
}

void
sort_modules(void)
{
    module *mod;

    discover_modules();

    /* ksymtab for startup.bin */
    init_symtable();

    list_for_each_entry(mod, &(pending), list) {
        analysis_module(mod);
    }
}

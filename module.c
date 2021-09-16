/*
 * Module
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "module.h"
#include "util.h"

#define PAGE_OFFSET 0xffffffe000000000UL

static LIST_HEAD(pending);
static LIST_HEAD(modules);

static LIST_HEAD(symbols);

static uint32_t ksym_ptr;
static uint32_t ksym_num;

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
            INIT_LIST_HEAD(&(mod->symbols));
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
export_symbols(const char *start, const char *end, list_head *sym_list)
{
    while (start < end) {
        symbol *sym = malloc(sizeof(symbol));
        sym->name = strdup(start);
        list_add_tail(&(sym->list), sym_list);

        pr_debug("%s: export(%s)\n", __func__, start);
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
    pr_debug("range(%x, %x)\n", ksym_ptr, ksym_num);

    fp = fopen(KMODULE_DIR "startup.bin", "rb");
    if (fp == NULL)
        panic("No System.map\n");

    fseek(fp, (long)ksym_ptr, SEEK_SET);

    buf = malloc(ksym_num);
    fread(buf, 1, ksym_num, fp);

    export_symbols(buf, buf + ksym_num, &symbols);

    free(buf);
    fclose(fp);
}

static char *
get_strtab(Elf64_Shdr *shdr, FILE *fp)
{
    char *str;

    str = (char *) malloc(shdr->sh_size);
    pr_debug("%s: offset(%lx)\n", __func__, shdr->sh_offset);

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
            pr_debug("%s: name(%s)\n", __func__, undef->name);
        }
    }

    free(sym);
}

static bool
match_undef(const char *name)
{
    symbol *sym;

    list_for_each_entry(sym, &(symbols), list) {
        if (strcmp(sym->name, name) == 0)
            return true;
    }

    return false;
}

static bool
check_dependency(module *mod)
{
    list_head *p, *n;

    list_for_each_safe(p, n, &(mod->undef_syms)) {
        symbol *undef = list_entry(p, symbol, list);
        if (match_undef(undef->name)) {
            list_del(&(undef->list));
            free(undef);
        }
    }

    if (list_empty(&(mod->undef_syms))) {
        list_splice_tail_init(&(mod->symbols), &(symbols));
        return true;
    }

    return false;
}

static bool
analysis_module(module *mod)
{
    int i;
    FILE *fp;
    Elf64_Shdr *sechdrs;
    char *secstrings;
    Elf64_Ehdr hdr = {0};
    char filename[256] = {0};

    pr_debug("%s: %s\n", __func__, mod->name);
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

            pr_debug("[%d]: SHT_SYMTAB (%lx, %lx)\n",
                     i, shdr->sh_offset, shdr->sh_size);

            discover_undef_syms(mod, shdr, strtab, fp);
            free(strtab);
        } else {
            const char *name = secstrings + shdr->sh_name;
            if (strcmp(name, "_ksymtab_strings") == 0) {
                char *ksym_str = get_strtab(shdr, fp);
                export_symbols(ksym_str, ksym_str + shdr->sh_size,
                               &mod->symbols);
            }
        }
    }

    free(secstrings);
    free(sechdrs);
    fclose(fp);

    return check_dependency(mod);
}

list_head *
sort_modules(void)
{
    list_head *p, *n;
    module *mod;
    symbol *sym;

    discover_modules();

    /* Init ksymtab based on startup.bin. */
    init_symtable();

    list_for_each_safe(p, n, &pending) {
        mod = list_entry(p, module, list);
        if (analysis_module(mod))
            list_move_tail(&(mod->list), &modules);
    }

    while (!list_empty(&pending)) {
        bool advance = false;
        list_for_each_safe(p, n, &pending) {
            mod = list_entry(p, module, list);
            if (check_dependency(mod)) {
                list_move_tail(&(mod->list), &modules);
                advance = true;
            }
        }

        if (!advance) {
            list_for_each_entry(mod, &pending, list) {
                list_for_each_entry(sym, &(mod->undef_syms), list) {
                    printf("undef '%s' in module '%s'.\n", sym->name, mod->name);
                }
            }

            panic("find undef symbols!\n");
        }
    }

    list_for_each_safe(p, n, &symbols) {
        sym = list_entry(p, symbol, list);
        list_del(&(sym->list));
        free(sym->name);
        free(sym);
    }

    return &modules;
}

void
clear_modules(void)
{
    list_head *p, *n;
    module *mod;

    list_for_each_safe(p, n, &modules) {
        mod = list_entry(p, module, list);
        list_del(&(mod->list));
        free(mod->name);
        free(mod);
    }
}

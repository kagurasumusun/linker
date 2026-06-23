#pragma once

#include <cstdint>

namespace fast_linker::macho {

struct mach_header {
    uint32_t magic;
    int32_t  cputype;
    int32_t  cpusubtype;
    uint32_t filetype;
    uint32_t ncmds;
    uint32_t sizeofcmds;
    uint32_t flags;
};

struct mach_header_64 {
    uint32_t magic;
    int32_t  cputype;
    int32_t  cpusubtype;
    uint32_t filetype;
    uint32_t ncmds;
    uint32_t sizeofcmds;
    uint32_t flags;
    uint32_t reserved;
};

inline constexpr uint32_t MH_MAGIC    = 0xfeedface;
inline constexpr uint32_t MH_CIGAM    = 0xcefaedfe;
inline constexpr uint32_t MH_MAGIC_64 = 0xfeedfacf;
inline constexpr uint32_t MH_CIGAM_64 = 0xcffaedfe;

struct load_command {
    uint32_t cmd;
    uint32_t cmdsize;
};

inline constexpr uint32_t LC_SEGMENT    = 0x1;
inline constexpr uint32_t LC_SYMTAB     = 0x2;
inline constexpr uint32_t LC_DYSYMTAB   = 0xb;
inline constexpr uint32_t LC_LOAD_DYLIB = 0xc;
inline constexpr uint32_t LC_ID_DYLIB   = 0xd;
inline constexpr uint32_t LC_UUID       = 0x1b;
inline constexpr uint32_t LC_SEGMENT_64 = 0x19;
inline constexpr uint32_t LC_MAIN       = 0x80000028;

struct segment_command {
    uint32_t cmd;
    uint32_t cmdsize;
    char     segname[16];
    uint32_t vmaddr;
    uint32_t vmsize;
    uint32_t fileoff;
    uint32_t filesize;
    int32_t  maxprot;
    int32_t  initprot;
    uint32_t nsects;
    uint32_t flags;
};

struct segment_command_64 {
    uint32_t cmd;
    uint32_t cmdsize;
    char     segname[16];
    uint64_t vmaddr;
    uint64_t vmsize;
    uint64_t fileoff;
    uint64_t filesize;
    int32_t  maxprot;
    int32_t  initprot;
    uint32_t nsects;
    uint32_t flags;
};

struct section {
    char     sectname[16];
    char     segname[16];
    uint32_t addr;
    uint32_t size;
    uint32_t offset;
    uint32_t align;
    uint32_t reloff;
    uint32_t nreloc;
    uint32_t flags;
    uint32_t reserved1;
    uint32_t reserved2;
};

struct section_64 {
    char     sectname[16];
    char     segname[16];
    uint64_t addr;
    uint64_t size;
    uint32_t offset;
    uint32_t align;
    uint32_t reloff;
    uint32_t nreloc;
    uint32_t flags;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
};

struct symtab_command {
    uint32_t cmd;
    uint32_t cmdsize;
    uint32_t symoff;
    uint32_t nsyms;
    uint32_t stroff;
    uint32_t strsize;
};

struct nlist {
    union {
        uint32_t n_strx;
    } n_un;
    uint8_t  n_type;
    uint8_t  n_sect;
    int16_t  n_desc;
    uint32_t n_value;
};

struct nlist_64 {
    union {
        uint32_t n_strx;
    } n_un;
    uint8_t  n_type;
    uint8_t  n_sect;
    uint16_t n_desc;
    uint64_t n_value;
};

inline constexpr uint8_t N_TYPE = 0x0e;
inline constexpr uint8_t N_UNDF = 0x0;
inline constexpr uint8_t N_ABS  = 0x2;
inline constexpr uint8_t N_SECT = 0xe;
inline constexpr uint8_t N_PEXT = 0x10;
inline constexpr uint8_t N_EXT  = 0x01;

struct fat_header {
    uint32_t magic;
    uint32_t nfat_arch;
};

struct fat_arch {
    int32_t  cputype;
    int32_t  cpusubtype;
    uint32_t offset;
    uint32_t size;
    uint32_t align;
};

#define FAT_MAGIC 0xcafebabe
#define FAT_CIGAM 0xbebafeca

struct dysymtab_command {
    uint32_t cmd;
    uint32_t cmdsize;
    uint32_t ilocalsym;
    uint32_t nlocalsym;
    uint32_t iextdefsym;
    uint32_t nextdefsym;
    uint32_t iundefsym;
    uint32_t nundefsym;
    uint32_t tocoff;
    uint32_t ntoc;
    uint32_t modtaboff;
    uint32_t nmodtab;
    uint32_t extrefsymoff;
    uint32_t nextrefsyms;
    uint32_t indirectsymoff;
    uint32_t nindirectsyms;
    uint32_t extreloff;
    uint32_t nextrel;
    uint32_t locreloff;
    uint32_t nlocrel;
};

struct relocation_info {
    int32_t  r_address;
    uint32_t r_symbolnum : 24,
             r_pcrel     : 1,
             r_length    : 2,
             r_extern    : 1,
             r_type      : 4;
};

// Chained Fixups
struct dyld_chained_fixups_header {
    uint32_t fixups_version;
    uint32_t starts_offset;
    uint32_t imports_offset;
    uint32_t symbols_offset;
    uint32_t imports_count;
    uint32_t imports_format;
    uint32_t symbols_format;
};

} // namespace fast_linker::macho

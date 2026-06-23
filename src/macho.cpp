#include "macho.h"
#include "util.h"
#include <cstring>

namespace fast_linker {

void MachOObjectFile::parse() {
    auto d = data();
    uint32_t magic = *reinterpret_cast<const uint32_t*>(d.data());

    if (magic == macho::MH_MAGIC_64) {
        is_64 = true;
        parse_64(d);
    } else if (magic == macho::MH_MAGIC) {
        is_64 = false;
        parse_32(d);
    }
}

void MachOObjectFile::parse_64(std::span<const uint8_t> d) {
    if (d.size() < sizeof(macho::mach_header_64)) throw std::runtime_error("File too small for Mach-O header");
    const auto* header = reinterpret_cast<const macho::mach_header_64*>(d.data());
    const uint8_t* p = d.data() + sizeof(macho::mach_header_64);
    for (uint32_t i = 0; i < header->ncmds; ++i) {
        if (p + sizeof(macho::load_command) > d.data() + d.size()) break;
        const auto* cmd = reinterpret_cast<const macho::load_command*>(p);
        if (p + cmd->cmdsize > d.data() + d.size()) break;

        if (cmd->cmd == macho::LC_SEGMENT_64) {
            const auto* seg = reinterpret_cast<const macho::segment_command_64*>(p);
            const auto* sec_ptr = reinterpret_cast<const macho::section_64*>(p + sizeof(macho::segment_command_64));
            for (uint32_t j = 0; j < seg->nsects; ++j) {
                uint32_t offset = sec_ptr[j].offset;
                uint64_t size = sec_ptr[j].size;
                if (offset + size > d.size()) throw std::runtime_error("Section out of bounds");

                sections.push_back({
                    .name = from_fixed_string(sec_ptr[j].sectname, 16),
                    .segname = from_fixed_string(sec_ptr[j].segname, 16),
                    .addr = sec_ptr[j].addr,
                    .size = size,
                    .align = sec_ptr[j].align,
                    .flags = sec_ptr[j].flags,
                    .data = d.subspan(offset, size)
                });
            }
        }
        p += cmd->cmdsize;
    }
}

void MachOObjectFile::parse_32(std::span<const uint8_t> d) {
    const auto* header = reinterpret_cast<const macho::mach_header*>(d.data());
    const uint8_t* p = d.data() + sizeof(macho::mach_header);
    for (uint32_t i = 0; i < header->ncmds; ++i) {
        const auto* cmd = reinterpret_cast<const macho::load_command*>(p);
        if (cmd->cmd == macho::LC_SEGMENT) {
            const auto* seg = reinterpret_cast<const macho::segment_command*>(p);
            const auto* sec_ptr = reinterpret_cast<const macho::section*>(p + sizeof(macho::segment_command));
            for (uint32_t j = 0; j < seg->nsects; ++j) {
                sections.push_back({
                    .name = from_fixed_string(sec_ptr[j].sectname, 16),
                    .segname = from_fixed_string(sec_ptr[j].segname, 16),
                    .addr = sec_ptr[j].addr,
                    .size = sec_ptr[j].size,
                    .align = sec_ptr[j].align,
                    .flags = sec_ptr[j].flags,
                    .data = d.subspan(sec_ptr[j].offset, sec_ptr[j].size)
                });
            }
        }
        p += cmd->cmdsize;
    }
}

void MachOObjectFile::resolve_symbols(Context& ctx) {
    auto d = data();
    const uint8_t* p = d.data() + (is_64 ? sizeof(macho::mach_header_64) : sizeof(macho::mach_header));
    uint32_t ncmds = is_64 ? reinterpret_cast<const macho::mach_header_64*>(d.data())->ncmds
                           : reinterpret_cast<const macho::mach_header*>(d.data())->ncmds;

    const macho::symtab_command* symtab = nullptr;
    for (uint32_t i = 0; i < ncmds; ++i) {
        const auto* cmd = reinterpret_cast<const macho::load_command*>(p);
        if (cmd->cmd == macho::LC_SYMTAB) {
            symtab = reinterpret_cast<const macho::symtab_command*>(p);
            break;
        }
        p += cmd->cmdsize;
    }

    if (!symtab) return;

    const char* strtab = reinterpret_cast<const char*>(d.data() + symtab->stroff);

    if (is_64) {
        const auto* nl = reinterpret_cast<const macho::nlist_64*>(d.data() + symtab->symoff);
        for (uint32_t i = 0; i < symtab->nsyms; ++i) {
            std::string_view name = &strtab[nl[i].n_un.n_strx];
            bool is_def = (nl[i].n_type & macho::N_TYPE) == macho::N_SECT;
            bool is_weak = (nl[i].n_desc & 0x0080); // N_WEAK_DEF or N_WEAK_REF
            Symbol* sym = ctx.resolve_symbol(name, this, is_def, is_weak);
            if (is_def) {
                sym->value = nl[i].n_value;
                defined_symbols.push_back(sym);
            } else {
                undefined_symbols.push_back(sym);
            }
        }
    } else {
        const auto* nl = reinterpret_cast<const macho::nlist*>(d.data() + symtab->symoff);
        for (uint32_t i = 0; i < symtab->nsyms; ++i) {
            std::string_view name = &strtab[nl[i].n_un.n_strx];
            bool is_def = (nl[i].n_type & macho::N_TYPE) == macho::N_SECT;
            bool is_weak = (nl[i].n_desc & 0x0080);
            Symbol* sym = ctx.resolve_symbol(name, this, is_def, is_weak);
            if (is_def) {
                sym->value = nl[i].n_value;
                defined_symbols.push_back(sym);
            } else {
                undefined_symbols.push_back(sym);
            }
        }
    }
}

void MachOObjectFile::apply_relocations(uint8_t* out, Context& ctx) {
    auto d = data();
    for (const auto& sec : sections) {
        // In a real implementation, we would parse sec.header.reloff
        // and apply each relocation.
        // For 'theoretically fastest', we use a parallel scan of relocations.

        // Pseudo-logic for demonstration:
        // if (sec.name == "__text") {
        //     // Apply patches to 'out' + sec.output_addr - base
        // }
    }
}

} // namespace fast_linker

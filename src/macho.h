#pragma once

#include "macho_structs.h"
#include "linker.h"

namespace fast_linker {

struct MachOSection {
    std::string_view name;
    std::string_view segname;
    uint64_t addr;
    uint64_t size;
    uint32_t align;
    uint32_t flags;
    std::span<const uint8_t> data;
    uint64_t output_addr = 0;
};

class MachOObjectFile : public InputFile {
public:
    MachOObjectFile(fs::path path) : InputFile(std::move(path), FileKind::MachO) {}

    void parse() override;
    void resolve_symbols(Context& ctx);

    std::span<MachOSection> get_sections() { return sections; }
    void apply_relocations(uint8_t* out, Context& ctx);

private:
    void parse_64(std::span<const uint8_t> d);
    void parse_32(std::span<const uint8_t> d);

    std::vector<MachOSection> sections;
    std::vector<Symbol*> defined_symbols;
    std::vector<Symbol*> undefined_symbols;
    bool is_64 = true;
};

} // namespace fast_linker

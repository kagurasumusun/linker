#include "linker.h"
#include "macho.h"
#include <iostream>
#include <cstring>

namespace fast_linker {

void Linker::run() {
    std::cout << "Starting high-speed link..." << std::endl;

    // Phase 1: Parallel Parse
    tbb::parallel_for(size_t(0), ctx_.input_files.size(), [&](size_t i) {
        ctx_.input_files[i]->parse();
    });

    // Phase 2: Parallel Symbol Resolution
    tbb::parallel_for(size_t(0), ctx_.input_files.size(), [&](size_t i) {
        auto& file = *ctx_.input_files[i];
        if (file.kind() == FileKind::MachO) {
            static_cast<MachOObjectFile&>(file).resolve_symbols(ctx_);
        }
    });

    // Phase 3: Dead Strip (GC) & ICF
    // (To be implemented: Parallel mark-and-sweep for sections)

    // Phase 4: Layout
    layout();

    // Phase 5: Relocations & Metadata Correction
    // apply_relocations is called inside write_output for zero-copy

    // Phase 6: Write Output
    write_output("a.out");

    std::cout << "Link completed successfully." << std::endl;
}

void Linker::apply_relocations(uint8_t* out) {
    // Parallel relocation processing
    tbb::parallel_for(size_t(0), ctx_.input_files.size(), [&](size_t i) {
        auto& file = *ctx_.input_files[i];
        if (file.kind() == FileKind::MachO) {
            static_cast<MachOObjectFile&>(file).apply_relocations(out, ctx_);
        }
    });
}

void Linker::layout() {
    uint64_t current_text_addr = 0x100000000 + 4096;
    uint64_t current_data_addr = 0x200000000; // Simplified data base

    for (auto& file_ptr : ctx_.input_files) {
        auto& file = *file_ptr;
        if (file.kind() == FileKind::MachO) {
            auto& macho = static_cast<MachOObjectFile&>(file);
            for (auto& sec : macho.get_sections()) {
                uint64_t& current_addr = sec.segname.starts_with("__TEXT") ? current_text_addr : current_data_addr;

                uint64_t align = 1 << sec.align;
                current_addr = (current_addr + align - 1) & ~(align - 1);
                sec.output_addr = current_addr;

                // Handle BSS (Zero-fill)
                if ((sec.flags & 0xff) == 0x1) { // S_ZEROFILL
                    // BSS only increments current_addr in VM, not necessarily file_size in some cases
                    // but for simplicity we treat it normally for now.
                }

                current_addr += sec.size;
            }
        }
    }
}

void Linker::write_output(const fs::path& output_path) {
    // Exact file size calculation
    uint64_t max_off = 4096;
    for (auto& file_ptr : ctx_.input_files) {
        if (file_ptr->kind() == FileKind::MachO) {
            auto& macho = static_cast<MachOObjectFile&>(*file_ptr);
            for (auto& sec : macho.get_sections()) {
                uint64_t off = sec.output_addr - 0x100000000;
                max_off = std::max(max_off, off + sec.size);
            }
        }
    }
    size_t file_size = (max_off + 4095) & ~4095; // Align to page

    int fd = open(output_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0755);
    if (fd < 0) return;
    ftruncate(fd, file_size);

    uint8_t* out = static_cast<uint8_t*>(mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (out == MAP_FAILED) { close(fd); return; }

    auto* header = reinterpret_cast<macho::mach_header_64*>(out);
    header->magic = macho::MH_MAGIC_64;
    header->cputype = 0x0100000c; // ARM64
    header->filetype = 0x2; // MH_EXECUTE
    header->ncmds = 1;
    header->sizeofcmds = sizeof(macho::segment_command_64);

    // Write a single segment command
    auto* seg = reinterpret_cast<macho::segment_command_64*>(out + sizeof(macho::mach_header_64));
    seg->cmd = macho::LC_SEGMENT_64;
    seg->cmdsize = sizeof(macho::segment_command_64);
    std::strcpy(seg->segname, "__TEXT");
    seg->vmaddr = 0x100000000;
    seg->vmsize = 0x1000;
    seg->fileoff = 0;
    seg->filesize = 0x1000;
    seg->maxprot = 7; // rwx
    seg->initprot = 5; // r-x

    // Parallel copy of section data
    tbb::parallel_for(size_t(0), ctx_.input_files.size(), [&](size_t i) {
        auto& file = *ctx_.input_files[i];
        if (file.kind() == FileKind::MachO) {
            auto& macho = static_cast<MachOObjectFile&>(file);
            for (auto& sec : macho.get_sections()) {
                // Determine file offset from output_addr (simplified)
                uint64_t file_off = sec.output_addr - 0x100000000;
                if (file_off + sec.size <= file_size) {
                    std::memcpy(out + file_off, sec.data.data(), sec.data.size());
                }
            }
        }
    });

    // Apply relocations directly on the output mmap
    apply_relocations(out);

    // Generate __LINKEDIT
    // (Symbol table, String table, Dysymtab generation logic)

    msync(out, file_size, MS_SYNC);
    munmap(out, file_size);
    close(fd);
}

} // namespace fast_linker

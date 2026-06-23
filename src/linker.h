#pragma once

#include "common.h"
#include <atomic>
#include <tbb/concurrent_vector.h>

namespace fast_linker {

struct Symbol {
    std::string_view name;
    uint64_t value = 0;
    bool is_defined = false;
    bool is_weak = false;
    bool is_common = false;
    uint32_t common_align = 0;
    uint32_t common_size = 0;
    class InputFile* file = nullptr;
};

struct Section {
    std::string_view name;
    std::span<const uint8_t> contents;
    uint64_t address = 0;
    uint32_t alignment = 1;
};

enum class FileKind {
    MachO,
    Archive,
    Fat
};

class InputFile {
public:
    explicit InputFile(fs::path path, FileKind kind)
        : path_(std::move(path)), kind_(kind), mmap_(path_) {}
    virtual ~InputFile() = default;

    std::span<const uint8_t> data() const { return mmap_.span(); }
    const fs::path& path() const { return path_; }
    FileKind kind() const { return kind_; }

    virtual void parse() = 0;

protected:
    fs::path path_;
    FileKind kind_;
    MemoryMappedFile mmap_;
};

class Context {
public:
    tbb::concurrent_hash_map<std::string_view, Symbol*> symbol_table;
    std::vector<std::unique_ptr<InputFile>> input_files;

    // Use a concurrent vector to keep symbols alive and call destructors at the end
    tbb::concurrent_vector<Symbol> symbols_storage;

    Symbol* resolve_symbol(std::string_view name, InputFile* file, bool is_def, bool is_weak) {
        tbb::concurrent_hash_map<std::string_view, Symbol*>::accessor acc;
        if (symbol_table.insert(acc, name)) {
            auto it = symbols_storage.push_back(Symbol{.name = name, .value = 0, .is_defined = is_def, .is_weak = is_weak, .file = file});
            acc->second = &(*it);
            return acc->second;
        }
        Symbol* sym = acc->second;
        if (is_def) {
            // Locking the symbol during resolution to handle concurrent strong-symbol definition attempts
            // Note: In the final version, we'll use a more granular lock or atomic CAS.
            if (!sym->is_defined || (sym->is_weak && !is_weak)) {
                sym->is_defined = true;
                sym->is_weak = is_weak;
                sym->file = file;
            } else if (sym->is_defined && !sym->is_weak && !is_weak) {
                // Potential duplicate symbol error
            }
        }
        return sym;
    }
};

class Linker {
public:
    Linker(Context& ctx) : ctx_(ctx) {}
    void run();

private:
    void layout();
    void apply_relocations(uint8_t* out);
    void write_output(const fs::path& output_path);

    Context& ctx_;
};

} // namespace fast_linker

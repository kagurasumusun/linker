#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <span>
#include <filesystem>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <tbb/parallel_for.h>
#include <tbb/concurrent_hash_map.h>
#include <mimalloc.h>

namespace fast_linker {

namespace fs = std::filesystem;

class MemoryMappedFile {
public:
    MemoryMappedFile(const fs::path& path) {
        int fd = open(path.c_str(), O_RDONLY);
        if (fd < 0) throw std::runtime_error("Failed to open file");

        struct stat st;
        fstat(fd, &st);
        size_ = st.st_size;

        data_ = static_cast<uint8_t*>(mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd, 0));
        close(fd);

        if (data_ == MAP_FAILED) throw std::runtime_error("Failed to mmap file");
    }

    ~MemoryMappedFile() {
        if (data_) munmap(data_, size_);
    }

    std::span<const uint8_t> span() const { return {data_, size_}; }

private:
    uint8_t* data_ = nullptr;
    size_t size_ = 0;
};

} // namespace fast_linker

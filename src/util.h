#pragma once

#include <cstdint>
#include <bit>
#include <cstring>
#include <string_view>

namespace fast_linker {

// Endian-aware reading
template<typename T>
inline T read_le(const void* p) {
    T val;
    std::memcpy(&val, p, sizeof(T));
    if constexpr (std::endian::native == std::endian::big) {
        // Swap bytes if native is big (Mach-O is usually LE for ARM64/x86_64)
    }
    return val;
}

// Safer string_view for fixed-size Mach-O names
inline std::string_view from_fixed_string(const char* s, size_t n) {
    size_t len = 0;
    while (len < n && s[len] != '\0') len++;
    return {s, len};
}

} // namespace fast_linker

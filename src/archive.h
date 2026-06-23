#pragma once

#include "linker.h"

namespace fast_linker {

class ArchiveFile : public InputFile {
public:
    using InputFile::InputFile;

    void parse() override {
        auto d = data();
        if (d.size() < 8 || std::memcmp(d.data(), "!<arch>\n", 8) != 0) {
            return;
        }
        // In a real implementation, we would parse the archive index (__.SYMDEF)
        // and only load required object files.
    }
};

} // namespace fast_linker

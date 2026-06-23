#include "linker.h"
#include "macho.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: fast-linker <input files...>" << std::endl;
        return 1;
    }

    fast_linker::Context ctx;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg.ends_with(".a")) {
            // ctx.input_files.push_back(std::make_unique<fast_linker::ArchiveFile>(argv[i]));
        } else {
            ctx.input_files.push_back(std::make_unique<fast_linker::MachOObjectFile>(argv[i]));
        }
    }

    fast_linker::Linker linker(ctx);
    linker.run();

    return 0;
}

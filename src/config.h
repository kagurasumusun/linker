#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

namespace fast_linker {

struct Config {
    std::vector<std::string> order_file;
    std::vector<std::string> exported_symbols;
};

class ConfigParser {
public:
    static Config parse_order_file(const std::string& path) {
        Config config;
        std::ifstream file(path);
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) config.order_file.push_back(line);
        }
        return config;
    }
};

} // namespace fast_linker

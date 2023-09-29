#pragma once
#include "libparse/Config.hpp"

namespace libparse {
    std::map<std::string, std::string> Types(void);
    std::string getTypeFile(std::map<std::string, std::string> types, std::string file);
}

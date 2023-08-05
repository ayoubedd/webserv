#pragma once

#include <string>

namespace libhttp {
  struct Uri {
    static const char ENC[21];
    static const std::string DEC[21];
    static const std::string::size_type LEN = 21;
    static std::string encode(const std::string &uri);
    static std::string decode(const std::string &uri);
  };
} // namespace libhttp

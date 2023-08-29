#pragma once

#include <map>
#include <string>

namespace libhttp {
  struct RequestTarget {
    std::string                        path;
    std::map<std::string, std::string> params;
    std::string                        anchor;

    void                                build(std::string &reqTarget);
    static const char                   ENC[21];
    static const std::string            DEC[21];
    static const std::string::size_type LEN = 21;

    static std::string encode(const std::string &uri);
    static std::string decode(const std::string &uri);
  };
} // namespace libhttp

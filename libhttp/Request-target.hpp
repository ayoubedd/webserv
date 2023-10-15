#pragma once

#include <map>
#include <string>

namespace libhttp {
  struct RequestTarget {
    std::string                        path;
    std::map<std::string, std::string> params;
    std::string                        anchor;
    std::string                        rawPramas;

    RequestTarget();

    void                                build(std::string &reqTarget);
    static const char                   ENC[21];
    static const std::string            DEC[21];
    static const std::string::size_type LEN = 21;

    static std::string encode(const std::string &uri);
    static std::string decode(const std::string &uri);
    static std::string getPathFromUrl(const std::string &url);
  };
} // namespace libhttp

#pragma once

#include "libhttp/Headers.hpp"
#include <vector>

namespace libhttp {
  struct MutlipartFormDataEntity {
    enum error {
      OK,
      MALFORMED,
      END,
      PART_HEADERS_MISSING,
    };
    libhttp::HeadersMap headers;
    std::vector<char> body;
    static std::vector<libhttp::MutlipartFormDataEntity> decode(const std::vector<char> &src,
                                                                const std::string &del);
  };
} // namespace libhttp

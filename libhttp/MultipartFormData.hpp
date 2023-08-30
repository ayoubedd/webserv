#pragma once

#include "libhttp/Headers.hpp"
#include <utility>
#include <vector>

namespace libhttp {
  struct MutlipartFormDataEntity {
    enum error {
      OK,
      MALFORMED,
      END,
      MISSING_HEADERS,
      CONTENT_DISPOSITION_MISSING,
    };
    libhttp::HeadersMap headers;
    std::vector<char>   body;

    static std::pair<error, std::vector<libhttp::MutlipartFormDataEntity> >
    decode(const std::vector<char> &src, const std::string &del);

    static libhttp::MutlipartFormDataEntity::error
    sanityCheck(const libhttp::MutlipartFormDataEntity &);
  };
} // namespace libhttp

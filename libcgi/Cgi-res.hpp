#pragma once

// #include "libhttp/Request-target.hpp"
#include "libhttp/Headers.hpp"
#include <vector>

namespace libcgi {
  struct CgiRespons {
    libhttp::HeadersMap headers;
    std::vector<char>   body;
  };
} // namespace libcgi

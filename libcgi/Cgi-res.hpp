#pragma once

// #include "libhttp/Request-target.hpp"
#include "libhttp/Headers.hpp"
#include <vector>

namespace libcgi {
  struct Respons {
    std::vector<char> cgiHeader;
    std::vector<char> sockBuff;
    bool              statusLineExists;
    // std::vector<char> body;

    enum error { OK, MALFORMED };

    Respons();
    error build();
    error cgiHeaderToHttpHeader(const std::string &h);
  };
} // namespace libcgi

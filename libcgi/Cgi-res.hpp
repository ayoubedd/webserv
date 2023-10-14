#pragma once

// #include "libhttp/Request-target.hpp"
#include "libhttp/Headers.hpp"
#include <vector>

namespace libcgi {
  struct Respons {
    std::vector<char>  cgiHeader;
    std::vector<char> *sockBuff;
    bool               statusLineExists;

    enum error { OK, MALFORMED };

    Respons();
    ~Respons();
    error build();
    void  write(const char *ptr, size_t len);
    error cgiHeaderToHttpHeader(const std::string &h);
    void  clean();
  };
} // namespace libcgi

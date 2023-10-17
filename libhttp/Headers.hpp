#pragma once

#include <iostream>
#include <map>
#include <string>

namespace libhttp {
  typedef std::map<std::string, std::string>  HeadersMap;
  typedef std::pair<std::string, std::string> HeaderPair;

  struct Headers {
    HeadersMap headers;

    // constants
    static const char *CONTENT_TYPE;
    static const char *TRANSFER_ENCODING;
    static const char *CONTENT_LENGTH;
    static const char *CONTENT_RANGE;
    static const char *HOST;

    void delHeader(std::string);
    std::string &operator[](std::string);
  };
} // namespace libhttp

std::ostream &operator<<(std::ostream &os, const libhttp::Headers &h);

#pragma once

#include <map>
#include <string>
#include <iostream>

namespace libhttp {
  typedef std::map<std::string, std::string> HeadersMap;
  typedef std::pair<std::string, std::string> HeaderPair;

  

  struct Headers {
    HeadersMap headers;
    static const char *Content_Range;
    void delHeader(std::string);
    std::string &operator[](std::string);
  };
} // namespace libhttp

std::ostream& operator<<(std::ostream &os, const libhttp::Headers &h);
#pragma once

#include <string>
#include <map>

namespace libhttp {
  typedef std::map<std::string, std::string> HeadersMap;
  typedef std::pair<std::string, std::string> HeaderPair;

  struct Headers {
    HeadersMap headers;

    void delHeader(std::string);
    std::string& operator[](int i);
  };
} // namespace libhttp

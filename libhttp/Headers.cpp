#include "libhttp/Headers.hpp"

void libhttp::Headers::delHeader(std::string header) { this->headers.erase(header); }
std::string &libhttp::Headers::operator[](std::string k) { return this->headers[k]; }

std::ostream &operator<<(std::ostream &os, const libhttp::Headers &h) {
  libhttp::HeadersMap::const_iterator start = h.headers.begin();
  while (start != h.headers.end()) {
    os << '\'' << start->first << '\'' << ": " << '\'' << start->second << '\'' << std::endl;
    start++;
  }
  return os;
}

const char *libhttp::Headers::Content_Range = "Content_Range";
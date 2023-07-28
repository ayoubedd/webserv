#include "libhttp/Headers.hpp"

void libhttp::Headers::delHeader(std::string header) { this->headers.erase(header); }
std::string &libhttp::Headers::operator[](std::string k) { return this->headers[k]; }

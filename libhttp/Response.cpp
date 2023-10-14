#include "libhttp/Response.hpp"
#include <vector>

libhttp::Response::Response() {
  fd = -1;
  bytesToServe = -1;
  readBytes = 0;
  doneReading = false;
  buffer = new std::vector<char>;
}

libhttp::Response::Response(std::vector<char> *vec) {
  fd = -1;
  bytesToServe = -1;
  readBytes = 0;
  doneReading = false;
  buffer = vec;
}

libhttp::Response::~Response(void) {
  delete buffer;
}


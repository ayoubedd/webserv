#include "libhttp/Redirect.hpp"

libhttp::Response *libhttp::redirect(const std::string &location) {
  libhttp::Response *response = new libhttp::Response();

  std::string rawHeaders("HTTP/1.1 308 Permanent Redirect\r\n"
                         "Location: " +
                         location +
                         "\r\n"
                         "Content-Length: 0\r\n\r\n");

  response->buffer.insert(response->buffer.begin(), rawHeaders.begin(), rawHeaders.end());
  return response;
}

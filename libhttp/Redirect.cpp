#include "libhttp/Redirect.hpp"

libhttp::Response *libhttp::redirect(const libparse::RouteProps *route) {
  libhttp::Response *response = new libhttp::Response();

  std::string rawHeaders("HTTP/1.1 307 Temporary Redirect\r\n"
                         "Location: " +
                         route->redir +
                         "\r\n"
                         "Content-Length: 0\r\n\r\n");

  response->buffer.insert(response->buffer.begin(), rawHeaders.begin(), rawHeaders.end());
  return response;
}

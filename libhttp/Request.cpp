#include "libhttp/Request.hpp"
#include <arpa/inet.h>

libhttp::Request::Request(sockaddr_in *clientAddr)
    : method()
    , reqTarget()
    , version()
    , headers()
    , body()
    , allBodyLen(0)
    , clientAddr(clientAddr)
    , state(libnet::READING_HEADERS){};

std::ostream &operator<<(std::ostream &os, const libhttp::Request &req) {
  char      ip4[INET_ADDRSTRLEN] = {0};
  short int port = 0;

  if (req.clientAddr)
    inet_ntop(AF_INET, &req.clientAddr->sin_addr, ip4, INET_ADDRSTRLEN);
  if (req.clientAddr)
    port = ntohs(req.clientAddr->sin_port);

  os << ip4 << ":" << port << std::endl;
  os << req.state << std::endl;
  os << "method: " << req.method << std::endl;
  os << "path: " << req.reqTarget.path << std::endl;
  os << "version: " << req.version << std::endl;
  os << req.headers << std::endl;
  os << "body: " << std::string(req.body.begin(), req.body.end()) << std::endl;
  return os;
}

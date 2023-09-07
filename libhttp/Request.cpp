#include "libhttp/Request.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>

libhttp::Request::Request()
    : method("")
    , reqTarget()
    , version("")
    , headers(Headers{})
    , body()
    , clientAddr(NULL)
    , allBodyLen(0)
    , state(libnet::READING_HEADERS){};

std::ostream &operator<<(std::ostream &os, const libhttp::Request &req) {
  char ip4[INET_ADDRSTRLEN];

  const char *ptr = inet_ntop(AF_INET, &req.clientAddr->sin_addr, ip4, INET_ADDRSTRLEN);

  os << ptr << ":" << ntohs(req.clientAddr->sin_port) << std::endl;
  os << "method: " << req.method << std::endl;
  os << "path: " << req.reqTarget.path << std::endl;
  os << "version: " << req.version << std::endl;
  os << req.headers << std::endl;
  os << "body: " << std::string(req.body.begin(), req.body.end()) << std::endl;
  return os;
}

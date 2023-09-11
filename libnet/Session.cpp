#include "libnet/Session.hpp"
#include <cstring>
#include <netinet/ip.h>
// #include <string.h>

libnet::Session::Session(int fd, sockaddr_in *clientAddr)
    : fd(fd)
    // , request(libhttp::Request())
    // , status(READING_HEADERS)
    , reader(fd, *clientAddr)
    , clientAddr(clientAddr) {
  // this->request.clientAddr = clientAddr;
}

#include "libnet/Session.hpp"
#include "libhttp/Writer.hpp"
#include <cstring>
#include <netinet/ip.h>
// #include <string.h>

libnet::Session::Session(int fd, sockaddr_in *clientAddr)
    : fd(fd)
    , reader(fd, *clientAddr)
    , writer(fd)
    , cgi(clientAddr)
    , clientAddr(clientAddr)
    , permitedIo(0) {}

bool libnet::Session::isNonBlocking(int perm) {
  if (permitedIo & perm)
    return true;
  return false;
}

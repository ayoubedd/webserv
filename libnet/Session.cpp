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
    , clientAddr(clientAddr) {}

bool libnet::Session::isNonBlocking(int perm) {
  if (perm & permitedIo)
    return true;
  return false;
}

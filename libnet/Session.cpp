#include "libnet/Session.hpp"
#include "libhttp/Writer.hpp"
#include <cstring>
#include <netinet/ip.h>
#include <unistd.h>
// #include <string.h>

libnet::Session::Session(int fd, sockaddr_in *clientAddr)
    : fd(fd)
    , reader(fd, *clientAddr)
    , writer(fd)
    , clientAddr(clientAddr)
    , destroy(false)
    , permitedIo(0) {
  transferEncoding = nullptr;
  multipart = nullptr;
  sizedPost = nullptr;
  cgi = nullptr;
}

libnet::Session::~Session() {
  if (transferEncoding != nullptr)
    delete transferEncoding;

  if (multipart != nullptr)
    delete multipart;

  if (sizedPost != nullptr)
    delete sizedPost;

  if (cgi != nullptr)
    delete cgi;

  close(fd);

  delete clientAddr;
}

bool libnet::Session::isNonBlocking(int perm) {
  if (permitedIo & perm)
    return true;
  return false;
}

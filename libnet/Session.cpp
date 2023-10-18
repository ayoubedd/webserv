#include "libnet/Session.hpp"
#include "core/Timer.hpp"
#include "libhttp/Writer.hpp"
#include <bits/types/struct_timeval.h>
#include <cstdlib>
#include <cstring>
#include <netinet/ip.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

libnet::Session::Session(int fd, sockaddr_in *clientAddr)
    : fd(fd)
    , reader(fd, *clientAddr)
    , writer(fd)
    , transferEncoding(NULL)
    , multipart(nullptr)
    , sizedPost(nullptr)
    , cgi(nullptr)
    , clientAddr(clientAddr)
    , destroy(false)
    , gracefulClose(false)
    , permitedIo(0) {
  WebServ::updateTime(&lastModified);
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

size_t timevalToMsec(struct timeval time) { return (time.tv_sec * 1000) + (time.tv_usec / 1000); }

bool libnet::Session::isSessionAcitve(size_t threshold) {
  struct timeval now;

  WebServ::updateTime(&now);

  if ((timevalToMsec(lastModified) + (threshold * 1000)) > timevalToMsec(now))
    return true;

  return false;
}

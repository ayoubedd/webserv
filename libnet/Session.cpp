#include "libnet/Session.hpp"
#include "core/Timer.hpp"
#include "libhttp/Writer.hpp"
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
    , multipart(NULL)
    , sizedPost(NULL)
    , cgi(NULL)
    , clientAddr(clientAddr)
    , destroy(false)
    , gracefulClose(false)
    , permitedIo(0) {
  WebServ::syncTime(&lastActivity);
  WebServ::syncTime(&cgiProcessingStart);
}

libnet::Session::~Session() {
  if (transferEncoding != NULL)
    delete transferEncoding;

  if (multipart != NULL)
    delete multipart;

  if (sizedPost != NULL)
    delete sizedPost;

  if (cgi != NULL)
    delete cgi;

  close(fd);

  delete clientAddr;
}

bool libnet::Session::isNonBlocking(int perm) {
  if (permitedIo & perm)
    return true;
  return false;
}

bool libnet::Session::isSessionActive(bool isCgiCheck) {
  struct timeval now;

  WebServ::syncTime(&now);

  if (isCgiCheck == true) // CGI check
    if (WebServ::timevalToMsec(now) >
        (WebServ::timevalToMsec(cgiProcessingStart) + (CGI_TIMEOUT * 1000)))
      return false;

  // Session check
  if (WebServ::timevalToMsec(now) >
      (WebServ::timevalToMsec(lastActivity) + (SESSION_IDLE_TIME * 1000)))
    return false;

  return true;
}

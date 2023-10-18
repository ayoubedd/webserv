#include "core/Timer.hpp"
#include "libnet/Net.hpp"
#include <cstdlib>
#include <sys/select.h>
#include <sys/time.h>

void WebServ::expireSessions(libnet::Sessions &sessions) {
  libnet::Sessions::iterator begin = sessions.begin();
  libnet::Sessions::iterator end = sessions.end();

  while (begin != end) {
    libnet::Session *session = begin->second;
    if (session->isSessionAcitve(1000))
      session->gracefulClose = true;
    begin++;
  }
}

void WebServ::updateTime(struct timeval *time) {
  if (gettimeofday(time, NULL) == -1) {
    std::cerr << "error: failure getting time of day" << std::endl;
    exit(EXIT_FAILURE);
  }
}

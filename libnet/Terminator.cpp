#include "libnet/Terminator.hpp"
#include "core/Timer.hpp"
#include <unistd.h>
#include <vector>

static void extractSessionsToClose(const libnet::Sessions &sessions, std::vector<int> &dst) {
  libnet::Sessions::const_iterator begin = sessions.begin();
  libnet::Sessions::const_iterator end = sessions.end();

  while (begin != end) {
    libnet::Session *session = begin->second;

    if (session->destroy == true)
      dst.push_back(session->fd);

    if (session->gracefulClose == true && session->writer.responses.empty() == true)
      dst.push_back(session->fd);

    if (session->isSessionAcitve(SESSION_IDLE_TIME) == false)
      session->gracefulClose = true;

    begin++;
  }
}

void libnet::Terminator::terminate(libnet::Sessions &sessions) {
  std::vector<int> sessionsToClose;

  extractSessionsToClose(sessions, sessionsToClose);

  std::vector<int>::iterator begin = sessionsToClose.begin();
  std::vector<int>::iterator end = sessionsToClose.end();

  while (begin != end) {
    libnet::Sessions::iterator iter = sessions.find(*begin);

    delete iter->second;
    sessions.erase(iter->first);

    begin++;
  }
}

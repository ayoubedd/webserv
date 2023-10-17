#include "libnet/Terminator.hpp"
#include <vector>

static void extractSessionsToClose(const libnet::Sessions         &sessions,
                                   std::vector<libnet::Session *> &dst) {
  libnet::Sessions::const_iterator begin = sessions.begin();
  libnet::Sessions::const_iterator end = sessions.end();

  while (begin != end) {
    libnet::Session *session = begin->second;

    if (session->destroy == true)
      dst.push_back(session);

    begin++;
  }
}

void libnet::Terminator::terminateSessions(std::vector<libnet::Session *> sessions) {
  std::vector<libnet::Session *>::iterator begin = sessions.begin();
  std::vector<libnet::Session *>::iterator end = sessions.end();

  while (begin != end) {
    delete *begin;
    begin++;
  }
}

void libnet::Terminator::terminate(libnet::Sessions &sessions) {
  std::vector<libnet::Session *> sessionsToClose;

  extractSessionsToClose(sessions, sessionsToClose);
  terminateSessions(sessionsToClose);
}

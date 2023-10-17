#include "libnet/Terminator.hpp"
#include <vector>

static void extractSessionsToClose(const libnet::Sessions &sessions, std::vector<int> &dst) {
  libnet::Sessions::const_iterator begin = sessions.begin();
  libnet::Sessions::const_iterator end = sessions.end();

  while (begin != end) {
    libnet::Session *session = begin->second;

    if (session->destroy == true)
      dst.push_back(session->fd);

    begin++;
  }
}

void libnet::Terminator::terminate(libnet::Sessions &sessions) {
  std::vector<int> sessionsToClose;

  extractSessionsToClose(sessions, sessionsToClose);

  std::vector<int>::iterator sessionsToCloseBegin = sessionsToClose.begin();
  std::vector<int>::iterator sessionsToClosEnd = sessionsToClose.end();

  while (sessionsToCloseBegin != sessionsToClosEnd) {
    libnet::Sessions::iterator sessionIter = sessions.find(*sessionsToCloseBegin);

    std::cout << "--" << std::endl;
    std::cout << "deleteting session "  << sessionIter->second->fd << std::endl;

    delete sessionIter->second;
    sessions.erase(sessionIter);

    sessionsToCloseBegin++;
  }
}

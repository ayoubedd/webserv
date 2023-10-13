#pragma once

#include "libnet/Session.hpp"
#include "libparse/Config.hpp"
#include <sys/select.h>
#include <vector>

namespace libnet {
  typedef std::map<int, Session *> Sessions;
  typedef std::vector<int>         Sockets;

  struct Netenv {
    Sockets  sockets;
    Sessions sessions;

    Sockets  readySockets;
    Sessions readySessions;

    void setupSockets(libparse::Config &domains);
    void prepFdSets(void);
    void awaitEvents(void);
    void acceptNewClients(void);
    void destroySession(libnet::Session *session);

  private:
    fd_set fdReadSet;
    fd_set fdWriteSet;

    int largestFd(void);
  };

  // void destroySession(int fd, Sessions *sessions);
} // namespace libnet

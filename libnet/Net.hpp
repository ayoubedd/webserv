#pragma once

#include "libnet/Session.hpp"
#include "libparse/Config.hpp"
#include <bits/types/struct_timeval.h>
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

    fd_set fdReadSet;
    fd_set fdWriteSet;

    void setupSockets(libparse::Config &domains);
    void prepFdSets(void);
    void awaitEvents(void);
    void acceptNewClients(void);
    void destroySession(libnet::Session *session);

    struct timeval timeHolder;
    bool           isSessionsTimming;

    int largestFd(void);

    void subscribeSessions();
    void subscribeSockets();
  };

  // void destroySession(int fd, Sessions *sessions);
} // namespace libnet

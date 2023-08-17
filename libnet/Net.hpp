#pragma once

#include "libnet/Session.hpp"
#include "libparse/Config.hpp"
#include <sys/select.h>
#include <vector>

namespace libnet {
	typedef std::map<int, Session> Sessions;
	typedef std::vector<int> Sockets;
  typedef std::map<int, Session*> ReadyClients;

  struct Netenv {
    Sockets readReadySockets;
		ReadyClients readyClients;
		Sessions sessions;

    void setupSockets(libparse::Domains &domains);
    void prepFdSets(void);
    void awaitEvents(void);
    void acceptNewClients(void);
		void destroySession(libnet::Session& session);

  private:
    fd_set fdReadSet;
    fd_set fdWriteSet;

		Sockets sockets;

    int largestFd(void);
  };

void destroySession(int fd, Sessions& sessions);
} // namespace libnet

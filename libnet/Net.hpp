#pragma once

#include "libnet/Session.hpp"
#include "libparse/Config.hpp"
#include <sys/select.h>
#include <vector>

namespace libnet {
	typedef std::map<int, Session> Sessions;
	typedef std::vector<int> Sockets;

  struct Netenv {
    Sockets readReadySockets;
		std::map<int, Session*> readyClients;
		Sessions sessions;

    void setupSockets(libparse::Domains &domains);
    void prepFdSets(void);
    void awaitEvents(void);
    void acceptNewClients(void);
    void dropFd(int fd);
		void destroySession(libnet::Session& session);

  private:
    fd_set fdReadSet;
    fd_set fdWriteSet;

		Sockets sockets;

    int largestFd(void);
  };

} // namespace libnet

#pragma once

#include "libparse/Config.hpp"
#include <sys/select.h>
#include <vector>

namespace libnet {
  struct Netenv {
    std::vector<int> readReadySockets;
    std::vector<int> readReadyClients;
    std::vector<int> exptReadyFds;

    void setupSockets(libparse::Domains &domains);
    void prepFdSets(void);
    void awaitEvents(void);
    void acceptNewClients(void);

  private:
    fd_set fdReadSet;
    fd_set fdWriteSet;
    fd_set fdExptSet;

    std::vector<int> sockets;
    std::vector<int> clients;

    int largestFd(void);
  };

} // namespace libnet
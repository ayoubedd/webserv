#pragma once

#include "libhttp/Reader.hpp"
#include "libhttp/Request.hpp"
#include "libnet/SessionState.hpp"
#include <netinet/in.h>
#include <queue>

namespace libnet {
  struct Session {
    Session(int fd, sockaddr_in *clientAddr);

    int fd;

    std::queue<libhttp::Request> requests;
    libhttp::Request             request;
    libnet::SessionState         status;
    libhttp::Reader              reader;
    sockaddr_in                  clientAddr;
  };
} // namespace libnet

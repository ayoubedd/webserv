#pragma once

#include "libhttp/Reader.hpp"
#include "libhttp/Request.hpp"
#include "libnet/SessionState.hpp"
#include <queue>

namespace libnet {
  struct Session {
    Session(int fd);

    int fd;

    std::queue<libhttp::Request> requests;
    libhttp::Request request;
    libnet::SessionState status;
    libhttp::Reader reader;
  };

  bool operator<(const libnet::Session &, const libnet::Session &);
  bool operator==(std::pair<const int, libnet::Session> pair, const libnet::Session &);
} // namespace libnet

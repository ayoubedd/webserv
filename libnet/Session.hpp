#pragma once

#include "libhttp/Reader.hpp"
#include "libnet/SessionState.hpp"

namespace libnet {
  struct Session {
    Session(int fd);

    int fd;

		libnet::sessionState status;
    libhttp::Reader Reader;
  };

  bool operator<(const libnet::Session &, const libnet::Session &);
  bool operator==(std::pair<const int, libnet::Session> pair, const libnet::Session &);
} // namespace libnet

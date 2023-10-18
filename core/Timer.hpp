#pragma once

#include "libnet/Net.hpp"
#include "libnet/Session.hpp"

namespace WebServ {
#define SESSION_IDLE_TIME 60 // Time in seconds, for a Sessions to be ildle in.

  void expireSessions(libnet::Sessions &);
  void updateTime(struct timeval *);
}; // namespace WebServ

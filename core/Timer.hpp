#pragma once

#include "libnet/Net.hpp"
#include "libnet/Session.hpp"

namespace WebServ {

#define SESSION_IDLE_TIME 60 // Time in seconds, for a Sessions to be ildle in.
#define CGI_TIMEOUT 20        // Time in seconds, for cgi response to be ready.

  void   expireSessions(libnet::Sessions &);
  void   syncTime(struct timeval *);
  size_t timevalToMsec(struct timeval time);
  size_t calcLeftTime(struct timeval event, size_t threshold);

}; // namespace WebServ

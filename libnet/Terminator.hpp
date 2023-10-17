#pragma once

#include "libnet/Net.hpp"
namespace libnet {
  struct Terminator {
    static void terminate(libnet::Sessions &Sessions);
    static void terminateSessions(std::vector<libnet::Session *> sessions);
  };
}; // namespace libnet

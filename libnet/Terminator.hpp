#pragma once

#include "libnet/Net.hpp"
namespace libnet {
  struct Terminator {
    static void terminate(libnet::Sessions &Sessions);
  };
}; // namespace libnet

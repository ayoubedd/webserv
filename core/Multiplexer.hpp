#pragma once

#include "libhttp/Status.hpp"
#include "libnet/Session.hpp"
#include "libparse/Config.hpp"

namespace libhttp {

  struct Mux {
    static void multiplexer(libnet::Session *session, const libparse::Config &config);
  };

}; // namespace libhttp

#pragma once

#include "libnet/Session.hpp"
#include "libparse/Config.hpp"

namespace libhttp {

  struct Mux {
    enum Error {
      OK,
      UNMATCHED_HANDLER,
      ERROR_400,
      ERROR_403,
      ERROR_404,
      ERROR_500,
      ERROR_501,
    };

    static Error multiplexer(libnet::Session *session, const libparse::Config &config);
  };

}; // namespace libhttp

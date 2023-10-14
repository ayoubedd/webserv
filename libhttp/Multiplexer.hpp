#pragma once

#include "libhttp/Status.hpp"
#include "libnet/Session.hpp"
#include "libparse/Config.hpp"

namespace libhttp {

  struct Mux {
    // Used by GET/POST/DELETE/CGI wrappers
    // to denote success or failure
    enum MuxHandlerError {
      OK,
      UNMATCHED_HANDLER,
      ERROR_400,
      ERROR_403,
      ERROR_404,
      ERROR_500,
      ERROR_501,
      ERROR_416,
      DONE,
    };

    static libhttp::Status::Code multiplexer(libnet::Session        *session,
                                             const libparse::Config &config);
  };

}; // namespace libhttp

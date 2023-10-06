#pragma once

#include "libnet/Session.hpp"
#include "libparse/Config.hpp"

namespace libhttp {

  enum MultiplexerError {
    OK,
    UNMATCHED_HANDLER
  };

  MultiplexerError multiplexer(libnet::Session *session, const libparse::Config &config);
}; // namespace libhttp

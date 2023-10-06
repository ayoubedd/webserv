#pragma once

#include "libnet/Session.hpp"
#include "libparse/Config.hpp"

namespace libhttp {
  void multiplexer(libnet::Session *session, const libparse::Config &config);
};

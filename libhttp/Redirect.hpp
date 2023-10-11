#pragma once

#include "libhttp/Response.hpp"
#include "libparse/Config.hpp"

namespace libhttp {
  libhttp::Response *redirect(const libparse::RouteProps *route);
};


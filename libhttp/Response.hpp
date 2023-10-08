#pragma once

#include "libnet/SessionState.hpp"
#include <sys/types.h>
#include <vector>

namespace libhttp {

  struct Response {
    Response();

    int               fd;
    std::vector<char> buffer;
    ssize_t           bytesToServe;
    ssize_t           readBytes;
    bool              doneReading;
  };
} // namespace libhttp

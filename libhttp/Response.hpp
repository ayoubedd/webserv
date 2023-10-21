#pragma once

#include <sys/types.h>
#include <vector>

namespace libhttp {

  struct Response {
    Response();
    ~Response();
    Response(std::vector<char> *);

    int               fd;
    std::vector<char> *buffer;
    ssize_t           bytesToServe;
    ssize_t           readBytes;
    bool              doneReading;
  };
} // namespace libhttp

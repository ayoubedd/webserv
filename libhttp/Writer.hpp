#pragma once

#include "libhttp/Response.hpp"
#include <queue>
#include <sys/types.h>

namespace libhttp {

  struct Writer {
    enum erorr {
      OK,
      ERORR_WRITTING_TO_FD,
      ERORR_READING_FROM_FD,
    };

    int                    sock;
    std::queue<Response *> responses;
    size_t                 readWriteBufferSize;

    Writer(int sock, int bufferSize = 65000);

    erorr write();
  };

} // namespace libhttp

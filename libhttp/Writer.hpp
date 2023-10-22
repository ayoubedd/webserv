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
      WRITTEN_NADA,
    };

    int                    sock;
    std::queue<Response *> responses;
    size_t                 readWriteBufferSize;

    Writer(int sock, int bufferSize = 1024);
    ~Writer();

    erorr write(bool);
  };

} // namespace libhttp

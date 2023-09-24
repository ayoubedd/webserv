#pragma once

#include "libhttp/Methods.hpp"
#include "libhttp/Response.hpp"
#include "libnet/Session.hpp"
#include <sys/types.h>

namespace libhttp {

struct Writer{
  enum erorr {
    OK,
    ERORR,
    WAITING,
    DONE
  };
  unsigned int          readBuffSize,writeBuffSize;
  unsigned int          start,end;
  bool                  isReady;
  off_t                 offSet;
  std::queue<Response *> responses;
  int                   fd;

  Writer(int fd,off_t offSet = -1 ,int bufferSize = 8192);
  off_t advanceOffSet(int fd, size_t start);
  erorr write();
};


}

#pragma once

#include "libhttp/Methods.hpp"
#include "libhttp/Response.hpp"
#include "libnet/Session.hpp"

namespace libhttp {

struct Writer{
  int                   fd;
  unsigned int          readBuffSize,writeBuffSize;
  std::queue<Response *> responses;

  // fuctions
  int advanceOffSet(int fd, size_t start);
  int read(int fd, char *buffer,int bufferSize);

};


}

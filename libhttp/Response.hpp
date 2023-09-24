#pragma once
#include <map>
#include "libhttp/Methods.hpp"
#include "libnet/SessionState.hpp"

namespace libhttp {

struct Response{
  int fd;
  std::vector<char > headers;
  std::vector<char > buffer;
  bool inReady;
  libnet::SessionState state;
  // void a() {
  //   char *a;
  //   buffer.insert(buffer.end(), a, a + n);
  //   send(fd, &buffer.front(), buffer.size(), 0);
  // }
};
}

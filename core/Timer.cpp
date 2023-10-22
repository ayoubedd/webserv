#include "core/Timer.hpp"
#include "libnet/Net.hpp"
#include <cstdlib>
#include <sys/select.h>
#include <sys/time.h>

size_t WebServ::timevalToMsec(struct timeval time) {
  return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}

void WebServ::syncTime(struct timeval *time) {
  if (gettimeofday(time, NULL) == -1) {
    std::cerr << "error: failure getting time of day" << std::endl;
    exit(EXIT_FAILURE);
  }
}
size_t WebServ::calcLeftTime(struct timeval then, size_t threshold) {
  size_t         spentTime;
  struct timeval now;

  WebServ::syncTime(&now);

  spentTime = WebServ::timevalToMsec(now) - WebServ::timevalToMsec(then);

  if (spentTime > threshold)
    return 0;

  return threshold - spentTime;
}

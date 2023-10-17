#include "core/Cleanup.hpp"
#include "libnet/Terminator.hpp"

static void extractSessionsFds(libnet::Sessions &src, std::vector<libnet::Session *> &dst) {
  libnet::Sessions::iterator begin = src.begin();
  libnet::Sessions::iterator end = src.end();

  while (begin != end) {
    dst.push_back(begin->second);
    begin++;
  }
}

void WebServ::cleanup(libnet::Netenv &net) {
  std::vector<libnet::Session *> sessionsToClose;

  extractSessionsFds(net.sessions, sessionsToClose);
  libnet::Terminator::terminateSessions(sessionsToClose);
}

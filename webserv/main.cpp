#include "libnet/Net.hpp"
#include "libparse/Config.hpp"
#include "libparse/utilities.hpp"

void sessionsHandler(libnet::Sessions &sessionsPool, libnet::ReadyClients &readySessions) {
  std::map<int, libnet::Session *>::iterator it;

  it = readySessions.begin();
  while (it != readySessions.end()) {
    it++;
  };
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "error: Missing config file \n";
    return 0;
  }

  libparse::Domains domains;
  libnet::Netenv net;

  libparse::parser(argv[1], domains);

  net.setupSockets(domains);

  while (true) {
    net.prepFdSets();
    net.awaitEvents();

    if (!net.readReadySockets.empty())
      net.acceptNewClients();

    sessionsHandler(net.sessions, net.readyClients);
  };

  return 0;
}

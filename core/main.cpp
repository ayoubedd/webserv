#include "libnet/Net.hpp"
#include "libparse/Config.hpp"
#include "libparse/utilities.hpp"

void sessionsHandler(libnet::Netenv &net) {
  libnet::Sessions::iterator session;
  libnet::Sessions          &readySessions = net.readyClients;

  session = readySessions.begin();
  while (session != readySessions.end()) {
    session++;
  };
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "error: Missing config file \n";
    return 0;
  }

  libparse::Config config;
  libnet::Netenv   net;

  libparse::parser(argv[1], config);

  net.setupSockets(config);

  while (true) {
    net.prepFdSets();
    net.awaitEvents();

    if (!net.readReadySockets.empty())
      net.acceptNewClients();

    sessionsHandler(net);
  };

  return 0;
}

#include "libhttp/Multiplexer.hpp"
#include "libnet/Net.hpp"
#include "libparse/Config.hpp"

void sessionshandler(libnet::Netenv &net, libparse::Domains &domains) {
  libnet::Sessions::iterator sessionIter;
  libnet::Sessions          &readysessions = net.readyClients;

  sessionIter = readysessions.begin();
  char buff[2];
  while (sessionIter != readysessions.end()) {
    // Temporary solutin for closed clients
    if (recv(sessionIter->second->fd, buff, 1, MSG_PEEK) <= 0) {
      sessionIter++;
      continue;
    }

    libnet::Session *session = sessionIter->second;

    // Calling the reader.
    session->reader.read();

    libhttp::multiplexer(session, domains);

    // should call the writter here.

    sessionIter++;
  };
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "error: missing config file \n";
    return 0;
  }

  libparse::Domains domains;
  libnet::Netenv    net;

  libparse::parser(argv[1], domains);

  net.setupSockets(domains);

  while (true) {
    net.prepFdSets();
    net.awaitEvents();

    if (!net.readReadySockets.empty())
      net.acceptNewClients();

    sessionshandler(net, domains);
  };
  return 0;
}

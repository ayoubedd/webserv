#include "libhttp/Multiplexer.hpp"
#include "libnet/Net.hpp"
#include "libparse/Config.hpp"
#include "libparse/utilities.hpp"
#include <assert.h>
#include <cstring>

void sessionsHandler(libnet::Netenv &net, libparse::Config &config) {
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

    libhttp::MultiplexerError muxErr = libhttp::multiplexer(session, config);

    if (muxErr != libhttp::MultiplexerError::OK) {
      // Call errors handler.
    }

    // Calling the writer.
    session->writer.write();

    sessionIter++;
  };
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "error: missing config file \n";
    return 0;
  }

  libparse::Config config;
  libnet::Netenv   net;

  libparse::parser(argv[1], config);

  if (!config.defaultServer) {
    std::cerr << "missing default server in the config" << std::endl;
    return 1;
  }

  net.setupSockets(config);

  while (true) {
    net.prepFdSets();
    net.awaitEvents();

    if (!net.readReadySockets.empty())
      net.acceptNewClients();

    sessionsHandler(net, config);
  };
  return 0;
}

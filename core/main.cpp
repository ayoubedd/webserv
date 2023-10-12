#include "libhttp/Multiplexer.hpp"
#include "libnet/Net.hpp"
#include "libparse/Config.hpp"
#include "libparse/utilities.hpp"
#include <assert.h>
#include <cstring>

void sessionsHandler(libnet::Netenv &net, libparse::Config &config) {
  libnet::Sessions::iterator sessionIter;
  libnet::Sessions          &readysessions = net.readySessions;

  sessionIter = readysessions.begin();
  char buff[2];
  while (sessionIter != readysessions.end()) {
    // Temporary solutin for closed clients
    if (recv(sessionIter->second->fd, buff, 1, MSG_PEEK) <= 0) {
      sessionIter++;
      continue;
    }

    libnet::Session *session = sessionIter->second;

    libhttp::Reader::error readerErr;
    libhttp::Writer::erorr writerError;
    libhttp::Status::Code  httpCode;

    // Calling the reader.
    if (session->isNonBlocking(libnet::Session::SOCK_READ))
      session->reader.read();

    httpCode = libhttp::Mux::multiplexer(session, config);

    // Calling the writer.
    if (session->isNonBlocking(libnet::Session::WRITER_READ | libnet::Session::SOCK_WRITE))
      session->writer.write();

    sessionIter++;
  };
}

int main(int argc, char *argv[]) {
  libparse::Config config;
  libnet::Netenv   net;

  if (argc < 2) {
    std::cerr << "error: missing config file \n";
    return 0;
  }

  libparse::parser(argv[1], config);

  if (!config.defaultServer) {
    std::cerr << "missing default server in the config" << std::endl;
    return 1;
  }

  net.setupSockets(config);

  while (true) {
    net.prepFdSets();
    net.awaitEvents();

    if (!net.readySockets.empty())
      net.acceptNewClients();

    sessionsHandler(net, config);
  };
  return 0;
}

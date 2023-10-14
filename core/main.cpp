#include "libhttp/Multiplexer.hpp"
#include "libnet/Net.hpp"
#include "libparse/Config.hpp"
#include "libparse/utilities.hpp"
#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

void sessionsHandler(libnet::Netenv &net, libparse::Config &config) {
  libnet::Sessions::iterator sessionsBegin = net.readySessions.begin();
  libnet::Sessions::iterator sessionsEnd = net.readySessions.end();

  while (sessionsBegin != sessionsEnd) {
    libnet::Session *session = sessionsBegin->second;

    libhttp::Reader::error readerErr = libhttp::Reader::OK;
    libhttp::Writer::erorr writerError = libhttp::Writer::OK;
    libhttp::Status::Code  httpCode = libhttp::Status::OK;

    // Calling the reader.
    if (session->isNonBlocking(libnet::Session::SOCK_READ))
      readerErr = session->reader.read();

    libhttp::Request *request = session->reader.requests.front();

    if (request->state == libnet::SessionState::READING_BODY ||
        request->state == libnet::SessionState::READING_FIN)
      httpCode = libhttp::Mux::multiplexer(session, config);

    // Calling the writer.
    if (session->isNonBlocking(libnet::Session::SOCK_WRITE))
      writerError = session->writer.write(session->isNonBlocking(libnet::Session::WRITER_READ));

    sessionsBegin++;
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

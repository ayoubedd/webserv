#include "core/Initialization.hpp"
#include "core/Logger.hpp"
#include "core/Multiplexer.hpp"
#include "core/Timer.hpp"
#include "libnet/Net.hpp"
#include "libnet/Terminator.hpp"
#include "libparse/Config.hpp"
#include "libparse/match.hpp"
#include "libparse/utilities.hpp"
#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

void sessionsHandler(libnet::Netenv &net, libparse::Config &config) {
  libnet::Sessions::iterator sessionsBegin = net.readySessions.begin();
  libnet::Sessions::iterator sessionsEnd = net.readySessions.end();

  while (sessionsBegin != sessionsEnd) {
    libnet::Session *session = sessionsBegin->second;

    libhttp::Reader::error readerErr = libhttp::Reader::OK;
    libhttp::Writer::erorr writerError = libhttp::Writer::OK;

    WebServ::updateTime(&session->lastActivity);

    // Calling the reader.
    if (session->isNonBlocking(libnet::Session::SOCK_READ)) {
      readerErr = session->reader.read();
      if (readerErr != libhttp::Reader::OK) {
        session->destroy = true;
        sessionsBegin++;
        continue;
      }
    }

    libhttp::Request *request = session->reader.requests.front();

    if (request->state == libhttp::Request::R_BODY || request->state == libhttp::Request::R_FIN)
      libhttp::Mux::multiplexer(session, config);

    // Calling the writer.
    if (session->isNonBlocking(libnet::Session::SOCK_WRITE)) {
      writerError = session->writer.write(session->isNonBlocking(libnet::Session::WRITER_READ));
      if (writerError != libhttp::Writer::OK) {
        session->destroy = true;
        sessionsBegin++;
        continue;
      }
    }

    sessionsBegin++;
  };
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Error: missing config file" << std::endl;
    return EXIT_FAILURE;
  }

  libparse::Config config;
  libnet::Netenv   net;

  // Calling parser, and check
  if (libparse::checkConfig(argv[1], config) == false)
    return EXIT_FAILURE;

  if (config.defaultServer == NULL) {
    std::cerr << "error: missing default server" << std::endl;
    return EXIT_FAILURE;
  }

  // Check for fs requirements
  if (WebServ::initializeFsEnv())
    return EXIT_FAILURE;

  // Initializing logs
  if (config.init() != true) {
    std::cerr << "error: failure initializing logging system" << std::endl;
    return EXIT_FAILURE;
  }

  // Initializing sockets
  net.setupSockets(config);

  while (true) {
    // Prepate fds sets / await for new events
    net.prepFdSets();
    net.awaitEvents();

    // Handing in-comming events
    sessionsHandler(net, config);

    // Accepting new clients
    if (net.readySockets.empty() != true)
      net.acceptNewClients();

    // Terminating sessions
    libnet::Terminator::terminate(net.sessions);
  };
  return 0;
}

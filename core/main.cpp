#include "libhttp/Chunk.hpp"
#include "libhttp/Methods.hpp"
#include "libhttp/Request.hpp"
#include "libnet/Net.hpp"
#include "libparse/Config.hpp"
#include <cstdio>
#include <cstdlib>
#include "libhttp/Methods.hpp"




void sessionshandler(libnet::Netenv &net) {
  libnet::Sessions::iterator session;
  libnet::Sessions          &readysessions = net.readyClients;

  session = readysessions.begin();
  char buff[2];
  while (session != readysessions.end()) {
    if (recv(session->second->fd, buff, 1, MSG_PEEK) <= 0) {
      session++;
      continue;
    }

    auto res = session->second->reader.read();

    if (res != libhttp::Reader::OK) {
      std::cout << "something is not going right" << std::endl;
    }

    if (session->second->reader.requests.front()->state == libnet::SessionState::READING_BODY ||
        session->second->reader.requests.front()->state == libnet::SessionState::READING_FIN) {
      std::cout << "method: " << session->second->reader.requests.front()->method << std::endl; 
      if(session->second->reader.requests.front()->method == "GET")
      {
        auto res=  libhttp::Get(*session->second->reader.requests.front(),
            "/home/amoujar/.zshrc");
        std::cout << "fd " << res.second.fd << " start : " << res.second.range.first
          << " end : " << res.second.range.second << " content-type : "<< res.second.contentType 
          <<"content-lenght: " << res.second.range.second - res.second.range.first << std::endl;
      }
      else if(session->second->reader.requests.front()->method == "DELETE")
      {
        auto res =   libhttp::Deletes(session->second->reader.requests.front()->reqTarget.path);
        std::cout << "status :" << res << std::endl;  
      }
      else  if(session->second->reader.requests.front()->method == "POST")
      {
        std::cout << "the reader is readhing the body" << std::endl;

      if (session->second->reader.requests.front()->state == libnet::SessionState::READING_FIN)
        std::cout << "the reader is done readying the body" << std::endl;

      libhttp::ChunkDecoder::ErrorStatusPair err;
      while (true) {
        std::cout << "RERUNING" << std::endl;
        err = session->second->transferEncoding.chunk.decoder.read(*session->second->reader.requests.front(),
                                                   "./upload");
        if (err.first == libhttp::ChunkDecoder::RERUN)
          continue;
        break;
      }

      if (err.first != libhttp::ChunkDecoder::OK) {
        std::cout << err.first << std::endl;
        std::cout << "chunk decoder failed" << std::endl;
        exit(1);
      }

      if (err.second == libhttp::ChunkDecoder::DONE) {
        std::cout << "chunk decoder done" << std::endl;
      }
    }
    }

    session++;
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

    sessionshandler(net);
  };
  return 0;
}


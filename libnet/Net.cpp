#include "libnet/Net.hpp"
#include <stdlib.h>
#include <cerrno>
#include <iostream>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <vector>

static int openSocket(std::string &port) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP
  hints.ai_flags = AI_PASSIVE; // Listening on all interfaces

  std::cerr << errno  << std::endl;

  struct addrinfo *addrinfo;
  if (getaddrinfo(NULL, port.c_str(), &hints, &addrinfo)) {
    std::cerr << "getaddrinfo" << strerror(errno)  << std::endl;
    exit(EXIT_FAILURE);
  }

  int sockfd = socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);
  if (sockfd == -1) {
    std::cerr << "socket" << strerror(errno)  << std::endl;
    exit(EXIT_FAILURE);
  }

  if (bind(sockfd, addrinfo->ai_addr, addrinfo->ai_addrlen)) {
    std::cerr << "bind" << strerror(errno)  << std::endl;
    exit(EXIT_FAILURE);
  }

  listen(sockfd, 1024);
  freeaddrinfo(addrinfo);

  return sockfd;
}

void libnet::Netenv::setupSockets(libparse::Domains &domains) {
  libparse::Domains::iterator begin = domains.begin();
  libparse::Domains::iterator end = domains.end();

  while (begin != end) {
    libparse::Domain domain = (*begin).second;

    // Opening / Pushing socket fd to sockets pool
    sockets.push_back(openSocket(domain.port));

    begin++;
  }
}

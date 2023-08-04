#include "libnet/Net.hpp"
#include <stdlib.h>
#include <algorithm>
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

static void insert_fds_into_fdset(std::vector<int> &vec, fd_set *set) {
  std::vector<int>::iterator begin = vec.begin();
  std::vector<int>::iterator end = vec.end();

  while (begin != end) {
    FD_SET(*begin, set);
    begin++;
  }
}

void libnet::Netenv::prepFdSets(void) {
  // Clear Sets for a new round
  FD_ZERO(&fdReadSet);
  FD_ZERO(&fdWriteSet);
  FD_ZERO(&fdExptSet);

  // Add Clients & Sockets fds to ReadSet
  insert_fds_into_fdset(clients, &fdReadSet);
  insert_fds_into_fdset(sockets, &fdReadSet);

  // Add Clients & Sockets fd to ExptSet
  insert_fds_into_fdset(clients, &fdExptSet);
  insert_fds_into_fdset(sockets, &fdExptSet);
}

int libnet::Netenv::largestFd(void) {
  std::vector<int>::iterator clientsLargestFd = std::max_element(clients.begin(), clients.end());
  std::vector<int>::iterator socketsLargestFd = std::max_element(sockets.begin(), sockets.end());

  if (clientsLargestFd == clients.end() && socketsLargestFd != sockets.end())
    return *socketsLargestFd;

  if (*clientsLargestFd > *socketsLargestFd)
    return *clientsLargestFd;
  else
    return *socketsLargestFd;

  return -1;
}

static void extract_matching_fds(std::vector<int> &src, std::vector<int> &dst, fd_set *set) {
  std::vector<int>::iterator begin = src.begin();
  std::vector<int>::iterator end = src.end();

  while (begin != end) {
    if (FD_ISSET(*begin, set))
      dst.push_back(*begin);
    begin++;
  }
}

void libnet::Netenv::awaitEvents(void) {
  int err = select(largestFd() + 1, &fdReadSet, &fdWriteSet, &fdExptSet, NULL);
  if (err == -1) {
    std::cerr << "select" << strerror(errno)  << std::endl;
    exit(EXIT_FAILURE);
  }

  // Clear Ready pools
  readReadySockets.clear();
  readReadyClients.clear();
  exptReadyFds.clear();

  // Extracing ready client & sockets into readyFdsPool
  extract_matching_fds(clients, readReadyClients, &fdReadSet);
  extract_matching_fds(sockets, readReadySockets, &fdReadSet);

  extract_matching_fds(clients, exptReadyFds, &fdExptSet);
  extract_matching_fds(sockets, exptReadyFds, &fdExptSet);
}

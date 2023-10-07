#include "libnet/Net.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <iostream>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

static int openSocket(std::string &port) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP
  hints.ai_flags = AI_PASSIVE;     // Listening on all interfaces

  struct addrinfo *addrinfo;
  if (getaddrinfo(NULL, port.c_str(), &hints, &addrinfo)) {
    std::cerr << "getaddrinfo" << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }

  int sockfd = socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);
  if (sockfd == -1) {
    std::cerr << "socket" << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }

  if (bind(sockfd, addrinfo->ai_addr, addrinfo->ai_addrlen)) {
    std::cerr << "bind" << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }

  listen(sockfd, 1024);
  freeaddrinfo(addrinfo);

  return sockfd;
}

void libnet::Netenv::setupSockets(libparse::Config &config) {
  libparse::Domains::iterator begin = config.domains.begin();
  libparse::Domains::iterator end = config.domains.end();

  while (begin != end) {
    libparse::Domain domain = begin->second;

    // Opening / Pushing socket fd to sockets pool
    sockets.push_back(openSocket(domain.port));

    begin++;
  }
}

static void insert_fds_into_fdset(libnet::Sockets &sockets, fd_set *set) {
  libnet::Sockets::iterator begin = sockets.begin();
  libnet::Sockets::iterator end = sockets.end();

  while (begin != end) {
    FD_SET(*begin, set);
    begin++;
  }
}

static void insert_fds_into_fdset(libnet::Sessions &sessions, fd_set *set) {
  libnet::Sessions::iterator begin = sessions.begin();
  libnet::Sessions::iterator end = sessions.end();

  while (begin != end) {
    FD_SET(begin->first, set);
    begin++;
  }
}
void libnet::Netenv::prepFdSets(void) {
  // Clear Sets for a new round
  FD_ZERO(&fdReadSet);
  FD_ZERO(&fdWriteSet);

  // Add Clients & Sockets fds to ReadSet
  insert_fds_into_fdset(sessions, &fdReadSet);
  insert_fds_into_fdset(sockets, &fdReadSet);
}

int libnet::Netenv::largestFd(void) {
  libnet::Sessions::iterator sessionsLargestFd = std::max_element(sessions.begin(), sessions.end());
  libnet::Sockets::iterator  socketsLargestFd = std::max_element(sockets.begin(), sockets.end());

  return std::max(sessionsLargestFd->first, *socketsLargestFd);
}

static void extract_matching_fds(libnet::Sessions &src, std::map<int, libnet::Session *> &dst,
                                 fd_set *set) {
  libnet::Sessions::iterator begin = src.begin();
  libnet::Sessions::iterator end = src.end();

  while (begin != end) {
    if (FD_ISSET(begin->first, set))
      dst.insert(std::make_pair(begin->first, begin->second));
    begin++;
  }
}

static void extract_matching_fds(libnet::Sockets &src, libnet::Sockets &dst, fd_set *set) {
  libnet::Sockets::iterator begin = src.begin();
  libnet::Sockets::iterator end = src.end();

  while (begin != end) {
    if (FD_ISSET(*begin, set))
      dst.push_back(*begin);
    begin++;
  }
}

void libnet::Netenv::awaitEvents(void) {
  int err = select(largestFd() + 1, &fdReadSet, &fdWriteSet, NULL, NULL);
  if (err == -1) {
    std::cerr << "select" << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }

  // Clear Ready pools
  readReadySockets.clear();
  readyClients.clear();

  // Extracing ready client & sockets into readyFdsPool
  extract_matching_fds(sessions, readyClients, &fdReadSet);
  extract_matching_fds(sockets, readReadySockets, &fdReadSet);
}

void libnet::Netenv::acceptNewClients(void) {
  libnet::Sockets::iterator begin = readReadySockets.begin();
  libnet::Sockets::iterator end = readReadySockets.end();
  sockaddr_in              *clientAddr;
  socklen_t                 clientAddrLen;

  int fd;
  clientAddrLen = sizeof *clientAddr;
  while (begin != end) {
    clientAddr = new sockaddr_in;
    if ((fd = accept(*begin, (sockaddr *)clientAddr, &clientAddrLen)) == -1) {
      std::cerr << "accept" << strerror(errno) << std::endl;
      return;
    }

    // add the new client to sessions pool
    sessions.insert(std::make_pair(fd, new libnet::Session(fd, clientAddr)));

    begin++;
  }
}

void libnet::Netenv::destroySession(Session *session) {
  libnet::Sessions::iterator sessionIter;

  sessionIter = this->sessions.find(session->fd);
  if (sessionIter == sessions.end())
    return;

  close(sessionIter->second->fd);
  delete sessionIter->second->clientAddr;
  delete sessionIter->second;
  sessions.erase(sessionIter);
}

#include "libnet/Net.hpp"
#include "libnet/Session.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
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
    std::cerr << "getaddrinfo: " << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }

  int sockfd = socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);
  if (sockfd == -1) {
    std::cerr << "socket: " << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }

  int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    std::cerr << "setsockopt: " << strerror(errno) << std::endl;
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  if (bind(sockfd, addrinfo->ai_addr, addrinfo->ai_addrlen)) {
    std::cerr << "bind: " << strerror(errno) << std::endl;
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

static void subscribeSockets(libnet::Sockets &sockets, fd_set *set) {
  libnet::Sockets::iterator begin = sockets.begin();
  libnet::Sockets::iterator end = sockets.end();

  while (begin != end) {
    FD_SET(*begin, set);
    begin++;
  }
}

static void subscribeSessions(libnet::Sessions &sessions, fd_set *fdReadSet, fd_set *fdWriteSet) {
  libnet::Sessions::iterator begin = sessions.begin();
  libnet::Sessions::iterator end = sessions.end();

  while (begin != end) {
    // Skip sessions with empty response queue
    libnet::Session *session = begin->second;

    // Always Subscribe for reading from the socket
    FD_SET(session->fd, fdReadSet);

    // Subscribe for reading from pipe if cgi in READING_HEADERS or READING_BODY state
    if (session->cgi)
      if (session->cgi->state == libcgi::Cgi::READING_HEADERS ||
          session->cgi->state == libcgi::Cgi::READING_BODY)
        FD_SET(session->cgi->fd[0], fdReadSet);

    // Subscribe for writting if there something to write
    if (session->writer.responses.empty() != true) {
      libhttp::Response *response = session->writer.responses.front();

      // Subscribe for writting
      FD_SET(session->fd, fdWriteSet);

      // Subscribe for reading if current response has a fd != -1
      // and not done reading
      if (response->fd != -1 && response->doneReading == false)
        FD_SET(response->fd, fdReadSet);
    }

    begin++;
  }
}

void libnet::Netenv::prepFdSets(void) {
  // Clear Sets for a new round
  FD_ZERO(&fdReadSet);
  FD_ZERO(&fdWriteSet);

  // Add Clients & Sockets fds to ReadSet
  subscribeSockets(sockets, &fdReadSet);
  subscribeSessions(sessions, &fdReadSet, &fdWriteSet);
}

int libnet::Netenv::largestFd(void) {
  libnet::Sockets::iterator socketsLargestFd = std::max_element(sockets.begin(), sockets.end());

  int largestFd = -1;

  libnet::Sessions::iterator sessionsBegin = sessions.begin();
  libnet::Sessions::iterator sessionsEnd = sessions.end();

  while (sessionsBegin != sessionsEnd) {
    libnet::Session *sesssion = sessionsBegin->second;

    // Comparing socket fd
    if (sesssion->fd > largestFd)
      largestFd = sesssion->fd;

    // If cgi active
    // check if cgi
    if (sesssion->cgi)
      if (sesssion->cgi->state == libcgi::Cgi::READING_HEADERS ||
          sesssion->cgi->state == libcgi::Cgi::READING_BODY)
        if (sesssion->cgi->fd[0] > largestFd)
          largestFd = sesssion->cgi->fd[0];

    if (sesssion->writer.responses.empty() == false)
      if (sesssion->writer.responses.front()->fd > largestFd)
        largestFd = sesssion->writer.responses.front()->fd;

    sessionsBegin++;
  }

  return std::max(largestFd, *socketsLargestFd);
}

static void extractReadySockets(libnet::Sockets &src, libnet::Sockets &dst, fd_set *set) {
  libnet::Sockets::iterator begin = src.begin();
  libnet::Sockets::iterator end = src.end();

  while (begin != end) {
    if (FD_ISSET(*begin, set))
      dst.push_back(*begin);
    begin++;
  }
}

static void extractReadySessions(libnet::Sessions &src, libnet::Sessions &dst, fd_set *fdReadSet,
                                 fd_set *fdWriteSet) {
  libnet::Sessions::iterator sessionsBegin = src.begin();
  libnet::Sessions::iterator sessionsEnd = src.end();

  while (sessionsBegin != sessionsEnd) {
    libnet::Session *session = sessionsBegin->second;

    // Rest permissions
    session->permitedIo = 0;

    // Check if allowed to read from socket
    if (FD_ISSET(session->fd, fdReadSet))
      session->permitedIo |= libnet::Session::SOCK_READ;

    if (session->writer.responses.empty() != true) {
      libhttp::Response *response = session->writer.responses.front();

      // Check if allowed to write to socket
      if (FD_ISSET(session->fd, fdWriteSet))
        session->permitedIo |= libnet::Session::SOCK_WRITE;

      // Check if writer allowed to read from response fd
      if (response->fd != -1 && FD_ISSET(response->fd, fdReadSet))
        session->permitedIo |= libnet::Session::WRITER_READ;
    }

    // Check if CGI allowed to read from pipe
    if (session->cgi)
      if ((session->cgi->state == libcgi::Cgi::READING_HEADERS ||
           session->cgi->state == libcgi::Cgi::READING_HEADERS) &&
          FD_ISSET(session->cgi->fd[0], fdReadSet))
        session->permitedIo |= libnet::Session::CGI_READ;

    // Telling if should pass this session to be handled
    if (session->permitedIo != 0)
      dst.insert(std::make_pair(session->fd, session));

    sessionsBegin++;
  }
}

void libnet::Netenv::awaitEvents(void) {
  int err = select(largestFd() + 1, &fdReadSet, &fdWriteSet, NULL, NULL);
  if (err == -1) {
    std::cerr << "select: " << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }

  // Clear Ready pools
  readySockets.clear();
  readySessions.clear();

  // Extracing ready client & sockets into readyFdsPool
  extractReadySockets(sockets, readySockets, &fdReadSet);
  extractReadySessions(sessions, readySessions, &fdReadSet, &fdWriteSet);
}

void libnet::Netenv::acceptNewClients(void) {
  libnet::Sockets::iterator begin = readySockets.begin();
  libnet::Sockets::iterator end = readySockets.end();
  sockaddr_in              *clientAddr;
  socklen_t                 clientAddrLen;

  int fd;
  clientAddrLen = sizeof *clientAddr;
  while (begin != end) {
    clientAddr = new sockaddr_in;
    if ((fd = accept(*begin, (sockaddr *)clientAddr, &clientAddrLen)) == -1) {
      std::cerr << "accept: " << strerror(errno) << std::endl;
      return;
    }

    // add the new client to sessions pool
    sessions.insert(std::make_pair(fd, new libnet::Session(fd, clientAddr)));

    begin++;
  }
}

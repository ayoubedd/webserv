#pragma once

#include "libcgi/Cgi.hpp"
#include "libhttp/Multipart.hpp"
#include "libhttp/Reader.hpp"
#include "libhttp/Request.hpp"
#include "libhttp/Sized-Post.hpp"
#include "libhttp/TransferEncoding.hpp"
#include "libhttp/Writer.hpp"
#include "libnet/SessionState.hpp"
#include <netinet/in.h>
#include <queue>

namespace libnet {
  struct Session {

    enum Permission {
      SOCK_READ = 1 << 0,   // Ability to read from socket fd
      SOCK_WRITE = 1 << 1,  // Ability to write to socket fd
      CGI_READ = 1 << 2,    // Ability of cgi to read from its read end of the pipe
      WRITER_READ = 1 << 3, // Ability of writer to read from its fd
    };

    Session(int fd, sockaddr_in *clientAddr);

    int                       fd;
    libhttp::TransferEncoding transferEncoding;
    libhttp::Multipart        multipart;
    libhttp::SizedPost        sizedPost;
    libhttp::Reader           reader;
    libhttp::Writer           writer;
    libcgi::Cgi               cgi;
    sockaddr_in              *clientAddr;
    bool                      destroy;

    int permitedIo;
    // Bit-maping
    // 1: Able to read from socket
    // 2: Able to write to socket
    // 3: CGI able to read from the pipe
    // 4: Writer able to read from its fd

    // Utility to check if the corresponding
    // io operation going to block or not
    bool isNonBlocking(int);

    void cleanup();
  };
} // namespace libnet

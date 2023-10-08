#pragma once

#include "libcgi/Cgi.hpp"
#include "libhttp/Multipart.hpp"
#include "libhttp/Reader.hpp"
#include "libhttp/Request.hpp"
#include "libhttp/TransferEncoding.hpp"
#include "libhttp/Writer.hpp"
#include "libnet/SessionState.hpp"
#include <netinet/in.h>
#include <queue>

namespace libnet {
  struct Session {
    Session(int fd, sockaddr_in *clientAddr);

    int                       fd;
    libhttp::TransferEncoding transferEncoding;
    libhttp::Multipart        multipart;
    libhttp::Reader           reader;
    libhttp::Writer           writer;
    libcgi::Cgi               cgi;
    sockaddr_in              *clientAddr;
  };
} // namespace libnet

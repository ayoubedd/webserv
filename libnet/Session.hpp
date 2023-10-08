#pragma once

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
    Session(int fd, sockaddr_in *clientAddr);

    int                       fd;
    libhttp::TransferEncoding transferEncoding;
    libhttp::Multipart        multipart;
    libhttp::SizedPost        sizedPost;
    libhttp::Reader           reader;
    libhttp::Writer           writer;
    sockaddr_in              *clientAddr;
  };
} // namespace libnet

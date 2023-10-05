#pragma once

#include "libhttp/Chunk.hpp"
#include "libhttp/Multipart.hpp"
#include "libhttp/Request.hpp"
#include "libhttp/TransferEncoding.hpp"

namespace libhttp {
  enum PostHandlerError {
    OK,
  };
  std::pair<libhttp::PostHandlerError, void *> postHandler(libhttp::Request &,
                                                           libhttp::TransferEncoding &,
                                                           libhttp::Multipart &,
                                                           const std::string &);
} // namespace libhttp

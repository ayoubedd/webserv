#pragma once

#include "libhttp/Chunk.hpp"
#include "libhttp/Multipart.hpp"
#include "libhttp/Request.hpp"
#include "libhttp/TransferEncoding.hpp"

namespace libhttp {
  enum PostHandlerError {
    OK,
    ERROR_400,
    ERROR_500,
  };

  std::pair<libhttp::PostHandlerError, libhttp::Response *> postHandler(libhttp::Request &,
                                                                        libhttp::TransferEncoding &,
                                                                        libhttp::Multipart &,
                                                                        const std::string &);
} // namespace libhttp

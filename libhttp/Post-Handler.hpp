#pragma once

#include "libhttp/Chunk.hpp"
#include "libhttp/Multipart.hpp"
#include "libhttp/Request.hpp"
#include "libhttp/TransferEncoding.hpp"

namespace libhttp {
  void postHandler(libhttp::Request &, libhttp::TransferEncoding &, libhttp::Multipart &,
                   const std::string &);
}

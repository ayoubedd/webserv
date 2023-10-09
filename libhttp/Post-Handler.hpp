#pragma once

#include "libhttp/Chunk.hpp"
#include "libhttp/Multipart.hpp"
#include "libhttp/Request.hpp"
#include "libhttp/Response.hpp"
#include "libhttp/Sized-Post.hpp"
#include "libhttp/TransferEncoding.hpp"

namespace libhttp {
  struct Post {
    enum Error {
      OK,
      ERROR_400,
      ERROR_500,
      DONE,
    };

    static std::pair<libhttp::Post::Error, libhttp::Response *>
    post(libhttp::Request &, libhttp::TransferEncoding &, libhttp::Multipart &,
         libhttp::SizedPost &sp, const std::string &);
  };

} // namespace libhttp

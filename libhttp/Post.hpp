#pragma once

#include "libhttp/Chunk.hpp"
#include "libhttp/Multipart.hpp"
#include "libhttp/Request.hpp"
#include "libhttp/Response.hpp"
#include "libhttp/Sized-Post.hpp"
#include "libhttp/TransferEncoding.hpp"

namespace libhttp {
  struct Post {
    enum Intel {
      OK,
      ERROR_400,
      ERROR_500,
      DONE,
    };

    enum BodyFormat {
      NORMAL,
      CHUNKED,
      MULTIPART_FORMDATA,
    };

    static std::pair<libhttp::Post::Intel, libhttp::Response *>
    post(libhttp::Request &, libhttp::TransferEncoding *, libhttp::Multipart *,
         libhttp::SizedPost *, const std::string &);

    static BodyFormat extractBodyFormat(const libhttp::HeadersMap &);
  };

} // namespace libhttp

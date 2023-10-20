#pragma once

#include "libhttp/Chunk.hpp"
#include "libhttp/Multipart.hpp"
#include "libhttp/Request.hpp"
#include "libhttp/Response.hpp"
#include "libhttp/Sized-Post.hpp"
#include "libhttp/Status.hpp"
#include "libhttp/TransferEncoding.hpp"

namespace libhttp {
  struct Post {
    enum BodyFormat {
      NORMAL,
      CHUNKED,
      MULTIPART_FORMDATA,
    };

    enum HandlerError {
      OK,
      ERROR_WRITTING_TO_FILE,
      ERROR_OPENING_FILE,
      ERROR_FILE_NOT_OPEN,
      BAD_REQUEST,
      DONE,
    };

    static std::pair<libhttp::Status::Code, libhttp::Response *>
    post(libhttp::Request &, libhttp::TransferEncoding *, libhttp::Multipart *,
         libhttp::SizedPost *, const std::string &);

    static BodyFormat extractBodyFormat(const libhttp::HeadersMap &);
  };

} // namespace libhttp

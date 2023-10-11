#pragma once

#include "libhttp/Headers.hpp"
#include "libhttp/Reader.hpp"
#include "libhttp/Request-target.hpp"
#include "libhttp/Request.hpp"
#include "libparse/Config.hpp"
#include <sys/types.h>

namespace WebServ {
  struct Sanitizer {
    enum Error {
      OK = 200,
      BAD_REQUEST = 400,
      NOT_FOUND = 404,
      METHOD_NOT_ALLOWED = 405,
      LENGTH_REQUIRED = 411,
      CONTENT_TOO_LARGE = 413,
      HTTP_VERSION_NOT_SUPPORTED = 505
    };
    static Error sanitizeMethod(const std::string &method, const libparse::RouteProps &route);
    static Error sanitizeReqLine(const libhttp::RequestTarget &reqTarget);
    static Error sanitizeHttpVersion(const std::string &httpVersion);
    static Error sanitizeHostHeader(const libhttp::HeadersMap &headers);
    // static Error sanitizeHeaderSize(const libhttp::Reader &reader, const ssize_t maxHeaderSize);
    static Error sanitizeBodySize(const libhttp::Request &req, const ssize_t maxBodySize);
    static Error sanitizeRequest(const libhttp::Request &req, const libparse::Config &config);

    static Error sanitizePostRequest(const libhttp::Request &req, const libparse::Config &config);
    static Error sanitizeGetRequest(const libhttp::Request &req, const libparse::Config &config);
  };

} // namespace WebServ
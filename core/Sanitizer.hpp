#pragma once

#include "libhttp/Headers.hpp"
#include "libhttp/Reader.hpp"
#include "libhttp/Request-target.hpp"
#include "libhttp/Request.hpp"
#include "libhttp/Status.hpp"
#include "libparse/Config.hpp"
#include <sys/types.h>

namespace WebServ {
  struct Sanitizer {
    typedef libhttp::Status Status;

    static Status::Code sanitizeMethod(const std::string          &method,
                                       const libparse::RouteProps &route);
    static Status::Code sanitizeReqLine(const libhttp::RequestTarget &reqTarget);
    static Status::Code sanitizeHttpVersion(const std::string &httpVersion);
    static Status::Code sanitizeHostHeader(const libhttp::HeadersMap &headers);
    static Status::Code sanitizeBodySize(const libhttp::Request &req, const ssize_t maxBodySize);
    static Status::Code sanitizeRequest(const libhttp::Request &req,
                                        const libparse::Domain &domain);

    static Status::Code sanitizePostRequest(const libhttp::Request &req,
                                            const libparse::Config &config);
    static Status::Code sanitizeGetRequest(const libhttp::Request &req,
                                           const libparse::Config &config);
  };

} // namespace WebServ

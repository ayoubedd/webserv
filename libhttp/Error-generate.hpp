#pragma once

#include "libhttp/Response.hpp"
#include "libhttp/Status.hpp"
#include "libparse/Config.hpp"
namespace libhttp {
  struct ErrorGenerator {

    static const char *OK;
    static const char *CREATED;
    static const char *PERMANENT_REDIRECT;
    static const char *BAD_REQUEST;
    static const char *UNAUTHORIZED;
    static const char *FORBIDDEN;
    static const char *NOT_FOUND;
    static const char *METHOD_NOT_ALLOWED;
    static const char *REQUEST_TIMEOUT;
    static const char *LENGTH_REQUIRED;
    static const char *PAYLOAD_TOO_LARGE;
    static const char *RANGE_NOT_SATISFIABLE;
    static const char *INTERNAL_SERVER_ERROR;
    static const char *NOT_IMPLEMENTED;
    static const char *GATEWAY_TIMEOUT;
    static const char *HTTP_VERSION_NOT_SUPPORTED;
    static const char *STATUS_CODE_VARIABLE;
    static const char *MESSAGE_VARIABLE;
    static const char *STATUS_LINE_TEMPLATE;
    static const char *SIZE;
    static const char *TITLE;

    static Response *generate(const libparse::Domain &domain, Status::Code code,
                              std::string defaultTemplate = "./static/error.html");
    static void      fileAsString(const std::string &filename,
                                  std::string       &buff); // filename should exitst of kbum

    static void fillTemplate(std::string &file, const std::map<std::string, std::string > arg);
  };
} // namespace libhttp

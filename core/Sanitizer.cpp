#include "core/Sanitizer.hpp"
#include "libhttp/Headers.hpp"
#include "libhttp/Methods.hpp"
#include "libhttp/Request.hpp"
#include "libparse/Config.hpp"
#include "libparse/match.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <utility>
#include <vector>

WebServ::Sanitizer::Error WebServ::Sanitizer::sanitizeMethod(const std::string          &method,
                                                             const libparse::RouteProps &route) {
  for (size_t i = 0; i < route.methods.size(); i++) {
    if (route.methods[i] == method)
      return OK;
  }
  return METHOD_NOT_ALLOWED;
}

WebServ::Sanitizer::Error
WebServ::Sanitizer::sanitizeReqLine(const libhttp::RequestTarget &reqTarget) {
  std::string::size_type i = reqTarget.path.find("..");
  if (i == std::string::npos)
    return OK;
  return BAD_REQUEST;
}

WebServ::Sanitizer::Error
WebServ::Sanitizer::sanitizeHostHeader(const libhttp::HeadersMap &headers) {
  const libhttp::HeadersMap::const_iterator host = headers.find(libhttp::Headers::HOST);
  if (host == headers.end())
    return BAD_REQUEST;
  return OK;
}

// WebServ::Sanitizer::Error WebServ::Sanitizer::sanitizeHeaderSize(const libhttp::Reader &reader,
//                                                                  const ssize_t maxHeaderSize) {}

WebServ::Sanitizer::Error WebServ::Sanitizer::sanitizeBodySize(const libhttp::Request &req,
                                                               const ssize_t maxBodySize) {
  if (maxBodySize == -1)
    return OK;
  if (req.body.size() > static_cast<std::vector<char>::size_type>(maxBodySize) ||
      req.allBodyLen > static_cast<std::vector<char>::size_type>(maxBodySize)) {
    return CONTENT_TOO_LARGE;
  }
  return OK;
}

WebServ::Sanitizer::Error WebServ::Sanitizer::sanitizePostRequest(const libhttp::Request &req,
                                                                  const libparse::Config &config) {
  if (req.method != libhttp::Methods::POST)
    return OK;
  std::string uploadDir = libparse::findUploadDir(req, *libparse::matchReqWithServer(req, config));
  if (uploadDir.empty())
    return NOT_FOUND;
  libhttp::HeadersMap::const_iterator it =
      req.headers.headers.find(libhttp::Headers::CONTENT_LENGTH);
  if (it != req.headers.headers.end())
    return OK;
  it = req.headers.headers.find(libhttp::Headers::CONTENT_TYPE);
  if (it != req.headers.headers.end())
    return OK;
  it = req.headers.headers.find(libhttp::Headers::TRANSFER_ENCODING);
  if (it != req.headers.headers.end())
    return OK;
  return LENGTH_REQUIRED;
}

WebServ::Sanitizer::Error WebServ::Sanitizer::sanitizeHttpVersion(const std::string &httpVersion) {
  if (httpVersion != "HTTP/1.1")
    return HTTP_VERSION_NOT_SUPPORTED;
  return OK;
}

WebServ::Sanitizer::Error WebServ::Sanitizer::sanitizeGetRequest(const libhttp::Request &req,
                                                                 const libparse::Config &config) {

  const libparse::Domain *d = libparse::matchReqWithServer(req, config);
  if (!d)
    return NOT_FOUND;

  std::pair<std::string, const libparse::RouteProps *> r =
      libparse::matchPathWithLocation(d->routes, req.reqTarget.path);
  if (!r.second)
    return NOT_FOUND;
  if (libparse::findResourceInFs(req, *d).empty())
    return NOT_FOUND;
  return OK;
}

WebServ::Sanitizer::Error WebServ::Sanitizer::sanitizeRequest(const libhttp::Request &req,
                                                              const libparse::Config &config) {
  Error                   e;
  const libparse::Domain *domain = libparse::matchReqWithServer(req, config);
  std::pair<std::string, const libparse::RouteProps * > route =
      libparse::matchPathWithLocation(domain->routes, req.reqTarget.path);

  e = sanitizeMethod(req.method, *route.second);
  if (e != OK)
    return e;
  return OK;
  e = sanitizeReqLine(req.reqTarget);
  if (e != OK)
    return e;

  e = sanitizeHttpVersion(req.version);
  if (e != OK)
    return e;

  e = sanitizeBodySize(req, domain->maxBodySize);
  if (e != OK)
    return e;
  return OK;
}
#include "core/Sanitizer.hpp"
#include "libhttp/Headers.hpp"
#include "libhttp/Methods.hpp"
#include "libhttp/Request.hpp"
#include "libparse/Config.hpp"
#include "libparse/match.hpp"
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <utility>
#include <vector>

WebServ::Sanitizer::Status::Code
WebServ::Sanitizer::sanitizeMethod(const std::string &method, const libparse::RouteProps &route) {
  for (size_t i = 0; i < route.methods.size(); i++) {
    if (route.methods[i] == method)
      return Status::OK;
  }
  return Status::METHOD_NOT_ALLOWED;
}

WebServ::Sanitizer::Status::Code
WebServ::Sanitizer::sanitizeReqLine(const libhttp::RequestTarget &reqTarget) {
  std::stringstream ss;
  std::string       tok;
  int               count = 0;
  ss << reqTarget.path;
  while (std::getline(ss, tok, '/')) {
    if (tok.empty())
      continue;
    if (tok == "..") {
      count--;
      continue;
    }
    count++;
  }
  if (count < 0)
    return Status::FORBIDDEN;
  return Status::OK;
}

WebServ::Sanitizer::Status::Code
WebServ::Sanitizer::sanitizeHostHeader(const libhttp::HeadersMap &headers) {
  const libhttp::HeadersMap::const_iterator host = headers.find(libhttp::Headers::HOST);
  if (host == headers.end())
    return Status::BAD_REQUEST;
  return Status::OK;
}

WebServ::Sanitizer::Status::Code WebServ::Sanitizer::sanitizeBodySize(const libhttp::Request &req,
                                                                      const ssize_t maxBodySize) {
  if (maxBodySize == -1)
    return Status::OK;
  if (req.body.size() > static_cast<std::vector<char>::size_type>(maxBodySize) ||
      req.allBodyLen > static_cast<std::vector<char>::size_type>(maxBodySize)) {
    return Status::PAYLOAD_TOO_LARGE;
  }
  return Status::OK;
}

WebServ::Sanitizer::Status::Code
WebServ::Sanitizer::sanitizePostRequest(const libhttp::Request &req,
                                        const libparse::Config &config) {
  if (req.method != libhttp::Methods::POST)
    return Status::OK;
  std::string uploadDir = libparse::findUploadDir(req, *libparse::matchReqWithServer(req, config));
  if (uploadDir.empty())
    return Status::NOT_FOUND;
  libhttp::HeadersMap::const_iterator it =
      req.headers.headers.find(libhttp::Headers::CONTENT_LENGTH);
  if (it != req.headers.headers.end())
    return Status::OK;
  it = req.headers.headers.find(libhttp::Headers::CONTENT_TYPE);
  if (it != req.headers.headers.end())
    return Status::OK;
  it = req.headers.headers.find(libhttp::Headers::TRANSFER_ENCODING);
  if (it != req.headers.headers.end())
    return Status::OK;
  return Status::LENGTH_REQUIRED;
}

WebServ::Sanitizer::Status::Code
WebServ::Sanitizer::sanitizeHttpVersion(const std::string &httpVersion) {
  if (httpVersion != "HTTP/1.1")
    return Status::HTTP_VERSION_NOT_SUPPORTED;
  return Status::OK;
}

WebServ::Sanitizer::Status::Code
WebServ::Sanitizer::sanitizeGetRequest(const libhttp::Request &req,
                                       const libparse::Config &config) {

  const libparse::Domain *d = libparse::matchReqWithServer(req, config);
  if (!d)
    return Status::NOT_FOUND;

  std::pair<std::string, const libparse::RouteProps *> r =
      libparse::matchPathWithRoute(d->routes, req.reqTarget.path);
  if (!r.second)
    return Status::NOT_FOUND;
  if (libparse::findResourceInFs(req, *d).empty())
    return Status::NOT_FOUND;
  return Status::OK;
}

WebServ::Sanitizer::Status::Code
WebServ::Sanitizer::sanitizeRequest(const libhttp::Request &req, const libparse::Domain &domain) {
  Status::Code                                          e;
  std::pair<std::string, const libparse::RouteProps * > route =
      libparse::matchPathWithRoute(domain.routes, req.reqTarget.path);

  e = sanitizeMethod(req.method, *route.second);
  if (e != Status::OK)
    return e;
  return Status::OK;
  e = sanitizeReqLine(req.reqTarget);
  if (e != Status::OK)
    return e;

  e = sanitizeHttpVersion(req.version);
  if (e != Status::OK)
    return e;

  return Status::OK;
}

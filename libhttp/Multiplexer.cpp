#include "libhttp/Multiplexer.hpp"
#include "libcgi/Cgi-res.hpp"
#include "libhttp/Headers.hpp"
#include "libhttp/Methods.hpp"
#include "libhttp/Post.hpp"
#include "libnet/Session.hpp"
#include "libparse/Config.hpp"
#include "libparse/match.hpp"
#include <cstdio>
#include <string>
#include <utility>

typedef std::pair<libhttp::Mux::Error, libhttp::Response *> MuxErrResPair;

static bool isRequestHandlerCgi(const libparse::RouteProps *route) {
  // if (route->cgi.second != "defautl path")
  //   return true;
  return false;
}

static MuxErrResPair deleteHandler(const std::string &path) {
  std::pair<libhttp::Methods::error, libhttp::Response> errResPair = libhttp::Delete(path);

  switch (errResPair.first) {
    case libhttp::Methods::FILE_NOT_FOUND:
      return std::make_pair(libhttp::Mux::ERROR_404, nullptr);
    case libhttp::Methods::FORBIDDEN:
      return std::make_pair(libhttp::Mux::ERROR_403, nullptr);
    case libhttp::Methods::OK:
      break;
  }

  libhttp::Response *response = new libhttp::Response(errResPair.second);

  return std::make_pair(libhttp::Mux::OK, response);
}

static MuxErrResPair getHandler(libhttp::Request &req, const std::string &path) {
  std::pair<libhttp::Methods::error, libhttp::Response> errResPair;

  errResPair = libhttp::Get(req, path);

  switch (errResPair.first) {
    case libhttp::Methods::FILE_NOT_FOUND:
      return std::make_pair(libhttp::Mux::ERROR_404, nullptr);
    case libhttp::Methods::FORBIDDEN:
      return std::make_pair(libhttp::Mux::ERROR_403, nullptr);
    case libhttp::Methods::OK:
      break;
  }

  libhttp::Response *res = new libhttp::Response(errResPair.second);

  return std::make_pair(libhttp::Mux::OK, res);
}

static MuxErrResPair postHandler(libhttp::Request &req, libhttp::Multipart &ml,
                                 libhttp::TransferEncoding &tr, libhttp::SizedPost &zp,
                                 const std::string &uploadRoot) {
  std::pair<libhttp::Post::Intel, libhttp::Response *> errResPair;

  errResPair = libhttp::Post::post(req, tr, ml, zp, uploadRoot);

  switch (errResPair.first) {
    case libhttp::Post::ERROR_400:
      return std::make_pair(libhttp::Mux::ERROR_400, nullptr);
    case libhttp::Post::ERROR_500:
      return std::make_pair(libhttp::Mux::ERROR_500, nullptr);
    case libhttp::Post::OK:
      return std::make_pair(libhttp::Mux::OK, nullptr);
    case libhttp::Post::DONE:
      break;
  }

  return std::make_pair(libhttp::Mux::OK, errResPair.second);
}

libhttp::Mux::Error libhttp::Mux::multiplexer(libnet::Session        *session,
                                              const libparse::Config &config) {
  libhttp::Request       *req = session->reader.requests.front();
  const libparse::Domain *domain = libparse::matchReqWithServer(*req, config);
  const std::pair<std::string, const libparse::RouteProps *> route =
      libparse::matchPathWithLocation(domain->routes, req->reqTarget.path);

  if (session->reader.requests.empty() == true)
    return libhttp::Mux::OK;

  MuxErrResPair errRes;

  if (isRequestHandlerCgi(route.second)) {

  }

  else if (req->method == "GET") {
    std::string resourcePath = libparse::findResourceInFs(*req, *domain);
    errRes = getHandler(*req, resourcePath);
  }

  else if (req->method == "DELETE") {
    std::string resourcePath = libparse::findResourceInFs(*req, *domain);
    errRes = deleteHandler(resourcePath);
  }

  else if (req->method == "POST") {
    std::string uploadRoot = libparse::findUploadDir(*req, *domain);
    errRes = postHandler(*req, session->multipart, session->transferEncoding, session->sizedPost,
                         uploadRoot);
  }

  if (errRes.first != OK)
    return errRes.first;

  if (errRes.second == nullptr)
    return libhttp::Mux::OK;

  // Response ready

  session->writer.responses.push(errRes.second);
  session->reader.requests.pop();

  return libhttp::Mux::UNMATCHED_HANDLER;
}

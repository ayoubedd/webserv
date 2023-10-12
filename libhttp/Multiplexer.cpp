#include "libhttp/Multiplexer.hpp"
#include "libcgi/Cgi-res.hpp"
#include "libhttp/Headers.hpp"
#include "libhttp/Methods.hpp"
#include "libhttp/Post.hpp"
#include "libhttp/Redirect.hpp"
#include "libhttp/Response.hpp"
#include "libnet/Session.hpp"
#include "libparse/Config.hpp"
#include "libparse/match.hpp"
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>

typedef std::pair<libhttp::Mux::Error, libhttp::Response *> MuxErrResPair;

static bool isRequestHandlerCgi(const libparse::RouteProps *route) {
  if (route->cgi.second != "defautl path")
    return true;
  return false;
}

static MuxErrResPair cgiHandler(libcgi::Cgi &cgi, const libparse::RouteProps *route,
                                libhttp::Request *req) {
  libcgi::Cgi::Error cgiError;

  cgiError = libcgi::Cgi::OK;

  switch (cgi.state) {
    case libcgi::Cgi::INIT:
      cgiError = cgi.init(req, route->cgi.second, "localhost", "./static/");
      if (cgiError != libcgi::Cgi::OK)
        break;
      cgi.exec();
    case libcgi::Cgi::READING_HEADERS:
    case libcgi::Cgi::READING_BODY:
      cgiError = cgi.read(); // Only read after passing through select or equivalent
      break;
    case libcgi::Cgi::ERR:
    case libcgi::Cgi::FIN:
      break;
  }

  switch (cgiError) {
    case libcgi::Cgi::FAILED_OPEN_FILE:
    case libcgi::Cgi::FAILED_OPEN_SCRIPT:
    case libcgi::Cgi::FAILED_EXEC_PERM:
    case libcgi::Cgi::FAILED_OPEN_PIPE:
    case libcgi::Cgi::FAILED_OPEN_DIR:
    case libcgi::Cgi::FAILED_FORK:
    case libcgi::Cgi::FAILED_WRITE:
    case libcgi::Cgi::FAILED_READ:
    case libcgi::Cgi::MALFORMED:
      return std::make_pair(libhttp::Mux::ERROR_500, nullptr);
    case libcgi::Cgi::OK:
      break;
  }

  if (cgi.state != libcgi::Cgi::READING_BODY && cgi.state != libcgi::Cgi::FIN)
    return std::make_pair(libhttp::Mux::OK, nullptr);

  // TODO:
  // - should create the response only one time
  // - and read till FIN
  libhttp::Response *response = new libhttp::Response();

  response->buffer = cgi.res.sockBuff;

  return std::make_pair(libhttp::Mux::OK, response);
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

  if (route.second->redir.empty() == false) {
    errRes.first = libhttp::Mux::OK;
    errRes.second = libhttp::redirect(route.second->redir);
  }

  else if (isRequestHandlerCgi(route.second)) {
    errRes = cgiHandler(session->cgi, route.second, req);
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

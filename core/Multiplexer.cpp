#include "core/Multiplexer.hpp"
#include "libcgi/Cgi-res.hpp"
#include "libhttp/Headers.hpp"
#include "libhttp/Methods.hpp"
#include "libhttp/Post.hpp"
#include "libhttp/Redirect.hpp"
#include "libhttp/Response.hpp"
#include "libnet/Session.hpp"
#include "libparse/Config.hpp"
#include "libparse/match.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>

typedef std::pair<libhttp::Mux::MuxHandlerError, libhttp::Response *> MuxErrResPair;

static bool isRequestHandlerCgi(const libparse::RouteProps *route) {
  if (route->cgi.second != "")
    return true;
  return false;
}

static MuxErrResPair cgiHandler(libcgi::Cgi *cgi, const libparse::RouteProps *route,
                                libhttp::Request *req) {
  libcgi::Cgi::Error cgiError;

  cgiError = libcgi::Cgi::OK;

  libcgi::Cgi::State prevState = cgi->state;

  switch (cgi->state) {
    case libcgi::Cgi::INIT:
      cgiError = cgi->init(req, route->cgi.second, "localhost", "./static/");
      if (cgiError != libcgi::Cgi::OK)
        break;
      cgiError = cgi->exec();
      break;
    case libcgi::Cgi::READING_HEADERS:
    case libcgi::Cgi::READING_BODY:
      std::cout << "cgi state: " << cgi->state << std::endl;
      cgiError = cgi->read();
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
    case libcgi::Cgi::FAILED_WAITPID:
    case libcgi::Cgi::CHIIED_RETURN_ERR:
      std::cout << "cgi failure" << std::endl;
      cgi->clean();
      return std::make_pair(libhttp::Mux::ERROR_500, nullptr);
    case libcgi::Cgi::OK:
      break;
  }
  std::cout << "-" << std::endl;

  if (cgi->state != libcgi::Cgi::READING_BODY && cgi->state != libcgi::Cgi::FIN)
    return std::make_pair(libhttp::Mux::OK, nullptr);

  // here and onward the state must be READING_BODY
  if (prevState == libcgi::Cgi::READING_HEADERS && cgi->state == libcgi::Cgi::READING_BODY) {
    libhttp::Response *response = new libhttp::Response(cgi->res.sockBuff);
    response->fd = -2;
    return std::make_pair(libhttp::Mux::OK, response);
  }

  if (cgi->state == libcgi::Cgi::FIN) {
    cgi->clean();
    return std::make_pair(libhttp::Mux::DONE, nullptr);
  }

  return std::make_pair(libhttp::Mux::OK, nullptr);
}

static MuxErrResPair deleteHandler(const std::string &path) {
  std::pair<libhttp::Methods::error, libhttp::Response *> errResPair = libhttp::Delete(path);

  switch (errResPair.first) {
    case libhttp::Methods::FILE_NOT_FOUND:
      return std::make_pair(libhttp::Mux::ERROR_404, nullptr);
    case libhttp::Methods::FORBIDDEN:
      return std::make_pair(libhttp::Mux::ERROR_403, nullptr);
    case libhttp::Methods::OUT_RANGE:
      return std::make_pair(libhttp::Mux::ERROR_416, nullptr);
    case libhttp::Methods::OK:
      break;
  }

  return std::make_pair(libhttp::Mux::DONE, errResPair.second);
}

static MuxErrResPair getHandler(libhttp::Request &req, const std::string &path) {
  std::pair<libhttp::Methods::error, libhttp::Response *> errResPair;

  errResPair = libhttp::Get(req, path);

  switch (errResPair.first) {
    case libhttp::Methods::FILE_NOT_FOUND:
      return std::make_pair(libhttp::Mux::ERROR_404, nullptr);
    case libhttp::Methods::FORBIDDEN:
      return std::make_pair(libhttp::Mux::ERROR_403, nullptr);
    case libhttp::Methods::OUT_RANGE:
      return std::make_pair(libhttp::Mux::ERROR_416, nullptr);
    case libhttp::Methods::OK:
      break;
  }

  return std::make_pair(libhttp::Mux::DONE, errResPair.second);
}

static MuxErrResPair postHandler(libhttp::Request &req, libhttp::Multipart *ml,
                                 libhttp::TransferEncoding *tr, libhttp::SizedPost *zp,
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

  return std::make_pair(libhttp::Mux::DONE, errResPair.second);
}

libhttp::Status::Code libhttp::Mux::multiplexer(libnet::Session        *session,
                                                const libparse::Config &config) {
  libhttp::Request       *req = session->reader.requests.front();
  const libparse::Domain *domain = libparse::matchReqWithServer(*req, config);
  const std::pair<std::string, const libparse::RouteProps *> route =
      libparse::matchPathWithRoute(domain->routes, req->reqTarget.path);

  MuxErrResPair errRes;

  if (route.second->redir.empty() == false) {
    errRes.first = libhttp::Mux::OK;
    errRes.second = libhttp::redirect(route.second->redir);
  }

  else if (isRequestHandlerCgi(route.second)) {
    if (session->cgi == nullptr)
      session->cgi = new libcgi::Cgi(session->clientAddr);
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
    libhttp::Post::BodyFormat bodyFormat = libhttp::Post::extractBodyFormat(req->headers.headers);

    switch (bodyFormat) {
      case Post::CHUNKED:
        if (session->transferEncoding == nullptr)
          session->transferEncoding = new libhttp::TransferEncoding();
        break;
      case Post::MULTIPART_FORMDATA:
        if (session->multipart == nullptr)
          session->multipart = new libhttp::Multipart();
        break;
      case Post::NORMAL:
        if (session->sizedPost == nullptr)
          session->sizedPost = new libhttp::SizedPost();
        break;
    }

    std::string uploadRoot = libparse::findUploadDir(*req, *domain);
    errRes = postHandler(*req, session->multipart, session->transferEncoding, session->sizedPost,
                         uploadRoot);
  }

  // Errors
  switch (errRes.first) {
    case UNMATCHED_HANDLER:
    case ERROR_400:
      return libhttp::Status::BAD_REQUEST;
    case ERROR_403:
      return libhttp::Status::FORBIDDEN;
    case ERROR_404:
      return libhttp::Status::NOT_FOUND;
    case ERROR_500:
      return libhttp::Status::INTERNAL_SERVER_ERROR;
    case ERROR_501:
      return libhttp::Status::NOT_IMPLEMENTED;
    case ERROR_416:
      return libhttp::Status::RANGE_NOT_SATISFIABLE;
    case DONE:
    case OK:
      break;
  }

  if (errRes.second != nullptr)
    session->writer.responses.push(errRes.second);

  if (errRes.first == DONE) {
    // Marking last resonse as done.
    if (session->writer.responses.back()->fd == -2) // if Respones is cgi
      session->writer.responses.back()->doneReading = true;
    // Pooping request since its done.
    session->reader.requests.pop();
  }

  // It might be other than ok in the actual response.
  // Rational:
  //  dons't matter what status code returned as long as denotes success.
  return libhttp::Status::OK;
}

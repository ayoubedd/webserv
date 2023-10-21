#include "core/Multiplexer.hpp"
#include "core/Logger.hpp"
#include "core/Sanitizer.hpp"
#include "core/Timer.hpp"
#include "libcgi/Cgi-res.hpp"
#include "libhttp/Error-generate.hpp"
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
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>

typedef std::pair<libhttp::Status::Code, libhttp::Response *> StatusResPair;

static std::pair<std::string, std::string>
extractInterpreterScriptPaths(const libparse::Domain *domain, const libparse::RouteProps *route,
                              libhttp::Request *req) {
  const std::string resourcePath = libparse::findResourceInFs(*req, *domain);

  if (resourcePath.empty() == true)
    return std::make_pair("", "");

  std::string::size_type idx = resourcePath.rfind(".");

  if (idx == std::string::npos)
    return std::make_pair("", "");

  if (idx == resourcePath.size())
    return std::make_pair("", "");

  idx++;

  std::string ext;

  try {
    ext = resourcePath.substr(idx);
  } catch (...) {
    return std::make_pair("", "");
  }

  std::map<std::string, std::string>::const_iterator cgi = route->cgi.find(ext);
  if (cgi == route->cgi.end())
    return std::make_pair("", resourcePath);

  return std::make_pair(cgi->second, resourcePath);
}

static bool shouldCloseSessions(libhttp::Request *request) {
  libhttp::HeadersMap::iterator iter = request->headers.headers.find(libhttp::Headers::CONNECTION);

  if (iter == request->headers.headers.end())
    return false;

  if (iter->second == "close")
    return true;

  return false;
}

static StatusResPair cgiHandler(libcgi::Cgi *cgi, const libparse::RouteProps *route,
                                const libparse::Domain *domain, libhttp::Request *req,
                                bool allowedToRead) {
  libcgi::Cgi::Error cgiError;

  cgiError = libcgi::Cgi::OK;

  libcgi::Cgi::State prevState = cgi->state;

  switch (cgi->state) {
    case libcgi::Cgi::INIT: {
      std::pair<std::string, std::string> interprterScriptPathsPair;

      interprterScriptPathsPair = extractInterpreterScriptPaths(domain, route, req);
      cgiError =
          cgi->init(req, interprterScriptPathsPair.second, "localhost", "./static/", domain->port);
      if (cgiError != libcgi::Cgi::OK)
        break;
      cgiError = cgi->exec(interprterScriptPathsPair.first);
      break;
    }
    case libcgi::Cgi::READING_HEADERS:
    case libcgi::Cgi::READING_BODY:
      if (allowedToRead == true)
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
      cgi->clean();
      return std::make_pair(libhttp::Status::INTERNAL_SERVER_ERROR,
                            static_cast<libhttp::Response *>(NULL));
    case libcgi::Cgi::CHIIED_RETURN_ERR:
    case libcgi::Cgi::OK:
      break;
  }

  if (cgi->state == libcgi::Cgi::FIN && prevState == libcgi::Cgi::READING_HEADERS) {
    delete cgi->res.sockBuff;
    cgi->clean();
    return std::make_pair(libhttp::Status::INTERNAL_SERVER_ERROR,
                          static_cast<libhttp::Response *>(NULL));
  }

  if (cgi->state != libcgi::Cgi::READING_BODY && cgi->state != libcgi::Cgi::FIN)
    return std::make_pair(libhttp::Status::OK, static_cast<libhttp::Response *>(NULL));

  // here and onward the state must be READING_BODY
  if (prevState == libcgi::Cgi::READING_HEADERS && cgi->state == libcgi::Cgi::READING_BODY) {
    libhttp::Response *response = new libhttp::Response(cgi->res.sockBuff);
    response->fd = -2;
    return std::make_pair(libhttp::Status::OK, response);
  }

  if (cgi->state == libcgi::Cgi::FIN) {
    cgi->clean();
    return std::make_pair(libhttp::Status::DONE, static_cast<libhttp::Response *>(NULL));
  }

  return std::make_pair(libhttp::Status::OK, static_cast<libhttp::Response *>(NULL));
}

static StatusResPair deleteHandler(const std::string &path) {
  std::pair<libhttp::Methods::error, libhttp::Response *> errResPair = libhttp::Delete(path);

  switch (errResPair.first) {
    case libhttp::Methods::FILE_NOT_FOUND:
      return std::make_pair(libhttp::Status::NOT_FOUND, static_cast<libhttp::Response *>(NULL));
    case libhttp::Methods::FORBIDDEN:
      return std::make_pair(libhttp::Status::FORBIDDEN, static_cast<libhttp::Response *>(NULL));
    case libhttp::Methods::OUT_RANGE:
      return std::make_pair(libhttp::Status::RANGE_NOT_SATISFIABLE,
                            static_cast<libhttp::Response *>(NULL));
    case libhttp::Methods::REDIR:
    case libhttp::Methods::OK:
      break;
  }

  return std::make_pair(libhttp::Status::DONE, errResPair.second);
}

static StatusResPair getHandler(libhttp::Request &req, const std::string &path) {
  std::pair<libhttp::Methods::error, libhttp::Response *> errResPair;

  errResPair = libhttp::Get(req, path);

  switch (errResPair.first) {
    case libhttp::Methods::FILE_NOT_FOUND:
      return std::make_pair(libhttp::Status::NOT_FOUND, static_cast<libhttp::Response *>(NULL));
    case libhttp::Methods::FORBIDDEN:
      return std::make_pair(libhttp::Status::FORBIDDEN, static_cast<libhttp::Response *>(NULL));
    case libhttp::Methods::OUT_RANGE:
      return std::make_pair(libhttp::Status::RANGE_NOT_SATISFIABLE,
                            static_cast<libhttp::Response *>(NULL));
    case libhttp::Methods::REDIR:
    case libhttp::Methods::OK:
      break;
  }

  return std::make_pair(libhttp::Status::DONE, errResPair.second);
}

static StatusResPair postHandler(libhttp::Request &req, libhttp::Multipart *ml,
                                 libhttp::TransferEncoding *tr, libhttp::SizedPost *zp,
                                 const std::string &uploadRoot) {
  std::pair<libhttp::Status::Code, libhttp::Response *> errResPair;

  errResPair = libhttp::Post::post(req, tr, ml, zp, uploadRoot);

  switch (errResPair.first) {
    case libhttp::Status::BAD_REQUEST:
      return std::make_pair(libhttp::Status::BAD_REQUEST, static_cast<libhttp::Response *>(NULL));
    case libhttp::Status::INTERNAL_SERVER_ERROR:
      return std::make_pair(libhttp::Status::INTERNAL_SERVER_ERROR,
                            static_cast<libhttp::Response *>(NULL));
    case libhttp::Status::OK:
      return std::make_pair(libhttp::Status::OK, static_cast<libhttp::Response *>(NULL));

    case libhttp::Status::CREATED:
    default:
      break;
  }

  return std::make_pair(libhttp::Status::DONE, errResPair.second);
}

static StatusResPair callCoresspondingHandler(libnet::Session *session, libhttp::Request *req,
                                              const libparse::Domain     *domain,
                                              const libparse::RouteProps *route) {
  StatusResPair errRes;

  errRes.first = libhttp::Status::OK;
  errRes.second = NULL;

  // Cgi
  if (route->cgi.size() != 0) {
    if (session->cgi == NULL)
      session->cgi = new libcgi::Cgi(session->clientAddr);

    // Checking if cgi time outed.
    if (session->cgi->state != libcgi::Cgi::INIT) // Only check in a running state
    {

      struct timeval now;
      WebServ::syncTime(&now);

      // Checking if cgi timeouted
      if (session->isSessionActive(true) == false) {

        // Cleaning up cgi
        session->cgi->clean();

        // denoting needed error to be generated
        errRes.first = libhttp::Status::GATEWAY_TIMEOUT;
        errRes.second = NULL;

        return errRes;
      }
    }

    // is cgi allowed to read
    bool allowedToRead = session->isNonBlocking(libnet::Session::CGI_READ);

    // Preserving cgi previous state
    libcgi::Cgi::State prevState = session->cgi->state;
    errRes = cgiHandler(session->cgi, route, domain, req, allowedToRead);

    // Telling if that previous cal to cgi was a new request processing
    if (prevState == libcgi::Cgi::INIT && session->cgi->state == libcgi::Cgi::READING_HEADERS)
      WebServ::syncTime(&session->cgiProcessingStart);
  }

  // Get
  else if (req->method == "GET") {
    std::string resourcePath = libparse::findResourceInFs(*req, *domain);
    errRes.first = WebServ::Sanitizer::sanitizeGetRequest(*req, *domain);
    if (errRes.first == libhttp::Status::OK)
      errRes = getHandler(*req, resourcePath);
  }

  // Delete
  else if (req->method == "DELETE") {
    std::string resourcePath = libparse::findResourceInFs(*req, *domain);
    errRes.first = WebServ::Sanitizer::sanitizeGetRequest(*req, *domain);
    if (errRes.first == libhttp::Status::OK)
      errRes = deleteHandler(resourcePath);
  }

  // Post
  else if (req->method == "POST") {
    libhttp::Post::BodyFormat bodyFormat = libhttp::Post::extractBodyFormat(req->headers.headers);

    switch (bodyFormat) {
      case libhttp::Post::CHUNKED:
        if (session->transferEncoding == NULL)
          session->transferEncoding = new libhttp::TransferEncoding();
        break;
      case libhttp::Post::MULTIPART_FORMDATA:
        if (session->multipart == NULL)
          session->multipart = new libhttp::Multipart();
        break;
      case libhttp::Post::NORMAL:
        if (session->sizedPost == NULL)
          session->sizedPost = new libhttp::SizedPost();
        break;
    }

    std::string uploadRoot = libparse::findUploadDir(&domain->routes, route);

    errRes.first = WebServ::Sanitizer::sanitizePostRequest(*req, domain->routes, *route);
    if (errRes.first == libhttp::Status::OK)
      errRes = postHandler(*req, session->multipart, session->transferEncoding, session->sizedPost,
                           uploadRoot);
  }

  return errRes;
}

void libhttp::Mux::multiplexer(libnet::Session *session, const libparse::Config &config) {

  libhttp::Request                                    *req;
  const libparse::Domain                              *domain;
  std::pair<std::string, const libparse::RouteProps *> route;
  StatusResPair                                        errRes;

  req = session->reader.requests.front();
  domain = libparse::matchReqWithServer(*req, config);
  route = libparse::matchPathWithRoute(domain->routes, req->reqTarget.path);

  errRes.first = libhttp::Status::OK;
  errRes.second = NULL;

  // Redirection
  if (route.second->redir.empty() == false) {
    errRes.first = libhttp::Status::DONE;
    errRes.second = libhttp::redirect(route.second->redir);
  }

  // Making sure we didn't exceed maxBodySize
  if (errRes.first == libhttp::Status::OK)
    if (session->isNonBlocking(libnet::Session::SOCK_READ))
      errRes.first = WebServ::Sanitizer::sanitizeBodySize(*req, domain->maxBodySize);

  // Request sanity check
  if (errRes.first == libhttp::Status::OK)
    if (req->sanitized == false) {
      errRes.first = WebServ::Sanitizer::sanitizeRequest(*req, *domain);
      req->sanitized = true;
    }

  // Calling implementations
  if (errRes.first == libhttp::Status::OK)
    errRes = callCoresspondingHandler(session, req, domain, route.second);

  // Errors
  switch (errRes.first) {
    case libhttp::Status::OK:
    case libhttp::Status::CREATED:
    case libhttp::Status::PARTIAL_CONTENT:
    case libhttp::Status::PERMANENT_REDIRECT:
    case libhttp::Status::DONE:
      break;

    default: {
      // Generating Error
      libhttp::Response *response = libhttp::ErrorGenerator::generate(*domain, errRes.first);
      errRes.first = libhttp::Status::DONE;
      errRes.second = response;
    }
  }

  if (errRes.second != NULL) {
    // Logggin
    Webserv::Logger::log(*req);
    session->writer.responses.push(errRes.second);
  }

  if (errRes.first == libhttp::Status::DONE) {
    // Marking last resonse as done.
    if (session->writer.responses.back()->fd == -2) // if Respones is cgi
      session->writer.responses.back()->doneReading = true;

    libhttp::Request *request = session->reader.requests.front();

    // if data still incoming stop listeining,
    // and flag session for graceful closing
    if (errRes.first != libhttp::Status::OK)
      if (request->state != libhttp::Request::R_FIN) {
        session->reader.clearRawDataIndices();
        session->gracefulClose = true;
      }

    // Flag session to gracefulClose if Connection: close is set
    if (shouldCloseSessions(request) == true)
      session->gracefulClose = true;

    // Pooping request since its done.
    delete session->reader.requests.front();
    session->reader.requests.pop();
  }
}

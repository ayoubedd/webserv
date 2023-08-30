#include "libcgi/cgi-req.hpp"
#include <algorithm>
#include <arpa/inet.h>

const char *libcgi::CgiRequest::AUTH_TYPE = "AUTH_TYPE";
const char *libcgi::CgiRequest::CONTENT_LENGTH = "CONTENT_LENGTH";
const char *libcgi::CgiRequest::CONTENT_TYPE = "CONTENT_TYPE";
const char *libcgi::CgiRequest::GATEWAY_INTERFACE = "GATEWAY_INTERFACE";
const char *libcgi::CgiRequest::PATH_INFO = "PATH_INFO";
const char *libcgi::CgiRequest::PATH_TRANSLATED = "PATH_TRANSLATED";
const char *libcgi::CgiRequest::QUERY_STRING = "QUERY_STRING";
const char *libcgi::CgiRequest::REMOTE_ADDR = "REMOTE_ADDR";
const char *libcgi::CgiRequest::REMOTE_HOST = "REMOTE_HOST";
const char *libcgi::CgiRequest::REMOTE_IDENT = "REMOTE_IDENT";
const char *libcgi::CgiRequest::REQUEST_METHOD = "REQUEST_METHOD";
const char *libcgi::CgiRequest::SCRIPT_NAME = "SCRIPT_NAME";
const char *libcgi::CgiRequest::SERVER_NAME = "SERVER_NAME";
const char *libcgi::CgiRequest::SERVER_PORT = "SERVER_PORT";
const char *libcgi::CgiRequest::SERVER_PROTOCOL = "SERVER_PROTOCOL";
const char *libcgi::CgiRequest::SERVER_SOFTWARE = "SERVER_SOFTWARE";

static std::string convertHeaderKey(std::string key) {
  std::transform(key.begin(), key.end(), key.begin(), ::toupper);
  if (key[0] == 'X' && key[1] == '-') {
    key[1] = '-';
    return key;
  }
  return "HTTP_" + key;
}

void libcgi::CgiRequest::convertReqHeadersToCgiHeaders(libhttp::Headers *httpHeaders) {
  CgiEnv::iterator it;

  it = httpHeaders->headers.begin();
  while (it != env.end()) {
    env.insert(std::make_pair(convertHeaderKey(it->first), it->second));
    it++;
  }
}

void libcgi::CgiRequest::addCgiStandardHeaders(libhttp::Request *httpReq) {
  char clientAddrBuff[INET_ADDRSTRLEN];
  ::inet_ntop(AF_INET, &httpReq->clientAddr->sin_addr, clientAddrBuff, INET_ADDRSTRLEN);

  env[AUTH_TYPE] = "";

  if (httpReq->headers.headers.find(CONTENT_LENGTH) != httpReq->headers.headers.end())
    env[CONTENT_LENGTH] = httpReq->headers[CONTENT_LENGTH];
  else
    env[CONTENT_LENGTH] = "";

  if (httpReq->headers.headers.find(CONTENT_TYPE) != httpReq->headers.headers.end())
    env[CONTENT_TYPE] = httpReq->headers[CONTENT_TYPE];
  else
    env[CONTENT_LENGTH] = "";

  env[GATEWAY_INTERFACE] = "CGI/1.1";
  env[PATH_INFO] = httpReq->reqTarget.path;
  env[QUERY_STRING] = httpReq->reqTarget.rawPramas;
  env[REMOTE_ADDR] = clientAddrBuff;
  env[REMOTE_HOST] = clientAddrBuff;
  env[SERVER_PORT] = ::ntohs(httpReq->clientAddr->sin_port);
  env[REMOTE_IDENT] = "";
  env[REQUEST_METHOD] = httpReq->method;
  env[SCRIPT_NAME] = ctx.scriptName;
  env[SERVER_NAME] = ctx.serverName;
  env[SERVER_PORT] = ctx.serverPort;
  env[SERVER_PROTOCOL] = ctx.protocol;
  env[SERVER_SOFTWARE] = ctx.serverSoftware;

  if (httpReq->reqTarget.path.size() > 0)
    env[PATH_TRANSLATED] = ctx.localReqPath;
}

void libcgi::CgiRequest::init(std::string serverName, std::string scriptName,
                              std::string localReqPath, std::string serverPort,
                              std::string protocol, std::string serverSoftware) {
  ctx.serverName = serverName;
  ctx.scriptName = scriptName;
  ctx.localReqPath = localReqPath;
  ctx.serverSoftware = serverPort;
  ctx.protocol = protocol;
  ctx.serverSoftware = serverSoftware;
}

void libcgi::CgiRequest::build(libhttp::Request *httpReq) {
  convertReqHeadersToCgiHeaders(&httpReq->headers);
  addCgiStandardHeaders(httpReq);
}

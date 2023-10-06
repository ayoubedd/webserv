#include "libcgi/Cgi-req.hpp"
#include <algorithm>
#include <arpa/inet.h>

const char *libcgi::Request::AUTH_TYPE = "AUTH_TYPE";
const char *libcgi::Request::CONTENT_LENGTH = "CONTENT_LENGTH";
const char *libcgi::Request::CONTENT_TYPE = "CONTENT_TYPE";
const char *libcgi::Request::GATEWAY_INTERFACE = "GATEWAY_INTERFACE";
const char *libcgi::Request::PATH_INFO = "PATH_INFO";
const char *libcgi::Request::PATH_TRANSLATED = "PATH_TRANSLATED";
const char *libcgi::Request::QUERY_STRING = "QUERY_STRING";
const char *libcgi::Request::REMOTE_ADDR = "REMOTE_ADDR";
const char *libcgi::Request::REMOTE_HOST = "REMOTE_HOST";
const char *libcgi::Request::REMOTE_IDENT = "REMOTE_IDENT";
const char *libcgi::Request::REQUEST_METHOD = "REQUEST_METHOD";
const char *libcgi::Request::SCRIPT_NAME = "SCRIPT_NAME";
const char *libcgi::Request::SERVER_NAME = "SERVER_NAME";
const char *libcgi::Request::SERVER_PORT = "SERVER_PORT";
const char *libcgi::Request::SERVER_PROTOCOL = "SERVER_PROTOCOL";
const char *libcgi::Request::SERVER_SOFTWARE = "SERVER_SOFTWARE";
const char *libcgi::Request::REDIRECT_STATUS = "REDIRECT_STATUS";

static std::string convertHeaderKey(std::string key) {
  std::transform(key.begin(), key.end(), key.begin(), ::toupper);
  if (key[0] == 'X' && key[1] == '-') {
    key[1] = '-';
    return key;
  }
  return "HTTP_" + key;
}

void libcgi::Request::convertReqHeadersToCgiHeaders(libhttp::Headers *httpHeaders) {
  libhttp::HeadersMap::const_iterator it, end;

  it = httpHeaders->headers.begin();
  end = httpHeaders->headers.end();
  while (it != end) {
    env.insert(std::make_pair(convertHeaderKey(it->first), it->second));
    it++;
  }
}

void libcgi::Request::addCgiStandardHeaders(libhttp::Request *httpReq) {
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
  env[REDIRECT_STATUS] = std::string("200");

  if (httpReq->reqTarget.path.size() > 0)
    env[PATH_TRANSLATED] = ctx.localReqPath;
}

void libcgi::Request::init(std::string scriptPath, std::string serverName, std::string scriptName,
                           std::string localReqPath, std::string serverPort, std::string protocol,
                           std::string serverSoftware) {
  this->scriptPath = scriptPath;
  ctx.serverName = serverName;
  ctx.scriptName = scriptName;
  ctx.localReqPath = localReqPath;
  ctx.serverSoftware = serverPort;
  ctx.protocol = protocol;
  ctx.serverSoftware = serverSoftware;
}

void libcgi::Request::build(libhttp::Request *httpReq) {
  convertReqHeadersToCgiHeaders(&httpReq->headers);
  addCgiStandardHeaders(httpReq);
}

void libcgi::Request::clean() {
  scriptPath.clear();
  env.clear();
  ctx.serverName.clear();
  ctx.scriptName.clear();
  ctx.protocol.clear();
  ctx.serverPort.clear();
  ctx.serverSoftware.clear();
  ctx.localReqPath.clear();
}

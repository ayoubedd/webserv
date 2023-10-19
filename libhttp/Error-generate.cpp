#include "libhttp/Error-generate.hpp"
#include "libhttp/Headers.hpp"
#include "libparse/Config.hpp"
#include <algorithm>
#include <assert.h>
#include <fstream>
#include <iterator>
#include <sstream>

const char *libhttp::ErrorGenerator::OK = "Ok";
const char *libhttp::ErrorGenerator::CREATED = "Created";
const char *libhttp::ErrorGenerator::PERMANENT_REDIRECT = "Permanent Redirect";
const char *libhttp::ErrorGenerator::BAD_REQUEST = "Bad Request";
const char *libhttp::ErrorGenerator::UNAUTHORIZED = "Unauthorized";
const char *libhttp::ErrorGenerator::FORBIDDEN = "Forbidden";
const char *libhttp::ErrorGenerator::NOT_FOUND = "Not Found";
const char *libhttp::ErrorGenerator::METHOD_NOT_ALLOWED = "Method Not Allowed";
const char *libhttp::ErrorGenerator::REQUEST_TIMEOUT = "Request Timeout";
const char *libhttp::ErrorGenerator::LENGTH_REQUIRED = "Length Required";
const char *libhttp::ErrorGenerator::PAYLOAD_TOO_LARGE = "Content Too Large";
const char *libhttp::ErrorGenerator::RANGE_NOT_SATISFIABLE = "Range Not Satisfiable";
const char *libhttp::ErrorGenerator::INTERNAL_SERVER_ERROR = " Internal Server Error";
const char *libhttp::ErrorGenerator::NOT_IMPLEMENTED = "Not Implemented";
const char *libhttp::ErrorGenerator::GATEWAY_TIMEOUT = "Gateway Timeout";
const char *libhttp::ErrorGenerator::HTTP_VERSION_NOT_SUPPORTED = "HTTP Version Not Supported";
const char *libhttp::ErrorGenerator::STATUS_CODE_VARIABLE = "{{STATUS_CODE}}";
const char *libhttp::ErrorGenerator::MESSAGE_VARIABLE = "{{MESSAGE_VARIABLE}}";
const char *libhttp::ErrorGenerator::STATUS_LINE_TEMPLATE =
    "HTTP/1.1 {{STATUS_CODE}} {{MESSAGE_VARIABLE}}\r\nContent-Type: text/html\r\nContent-Length: "
    "{{SIZE}}\r\n\r\n";
const char *libhttp::ErrorGenerator::SIZE = "{{SIZE}}";
const char *libhttp::ErrorGenerator::TITLE = "{{TITLE}}";

void libhttp::ErrorGenerator::fileAsString(const std::string &filename, std::string &buff) {
  std::ifstream     file;
  std::stringstream ss;

  file.open(filename);
  if (!file.is_open())
    return;
  ss << file.rdbuf();
  file.close();
  buff = ss.str();
}
void libhttp::ErrorGenerator::fillTemplate(std::string                              &file,
                                           const std::map<std::string, std::string > arg) {
  std::map<std::string, std::string>::const_iterator it;
  it = arg.begin();
  while (it != arg.end()) {
    std::string::size_type found;
    while ((found = file.find(it->first)) != std::string::npos) {
      file.replace(found, it->first.size(), it->second);
    }
    it++;
  }
}

static std::string asStr(size_t i) {
  std::stringstream ss;

  ss << i;
  return ss.str();
}

libhttp::Response *libhttp::ErrorGenerator::generate(const libparse::Domain &domain,
                                                     libhttp::Status::Code   code,
                                                     std::string             defaultTemplate) {
  if (code < libhttp::Status::BAD_REQUEST || code > libhttp::Status::HTTP_VERSION_NOT_SUPPORTED)
    return NULL;
  std::string filename = domain.error.empty() ? std::string(defaultTemplate) : domain.error;
  std::string file, status = STATUS_LINE_TEMPLATE;
  fileAsString(filename, file);
  if (file.empty())
    return NULL;
  Response                          *r = new Response;
  std::map<std::string, std::string> arg;
  arg[STATUS_CODE_VARIABLE] = asStr(code);
  switch (code) {
    case libhttp::Status::BAD_REQUEST:
      arg[MESSAGE_VARIABLE] = BAD_REQUEST;
      break;
    case libhttp::Status::UNAUTHORIZED:
      arg[MESSAGE_VARIABLE] = UNAUTHORIZED;
      break;
    case libhttp::Status::FORBIDDEN:
      arg[MESSAGE_VARIABLE] = FORBIDDEN;
      break;
    case libhttp::Status::NOT_FOUND:
      arg[MESSAGE_VARIABLE] = NOT_FOUND;
      break;
    case libhttp::Status::METHOD_NOT_ALLOWED:
      arg[MESSAGE_VARIABLE] = METHOD_NOT_ALLOWED;
      break;
    case libhttp::Status::REQUEST_TIMEOUT:
      arg[MESSAGE_VARIABLE] = REQUEST_TIMEOUT;
      break;
    case libhttp::Status::LENGTH_REQUIRED:
      arg[MESSAGE_VARIABLE] = LENGTH_REQUIRED;
      break;
    case libhttp::Status::PAYLOAD_TOO_LARGE:
      arg[MESSAGE_VARIABLE] = PAYLOAD_TOO_LARGE;
      break;
    case libhttp::Status::RANGE_NOT_SATISFIABLE:
      arg[MESSAGE_VARIABLE] = RANGE_NOT_SATISFIABLE;
      break;
    case libhttp::Status::INTERNAL_SERVER_ERROR:
      arg[MESSAGE_VARIABLE] = INTERNAL_SERVER_ERROR;
      break;
    case libhttp::Status::NOT_IMPLEMENTED:
      arg[MESSAGE_VARIABLE] = NOT_IMPLEMENTED;
      break;
    case libhttp::Status::GATEWAY_TIMEOUT:
      arg[MESSAGE_VARIABLE] = GATEWAY_TIMEOUT;
      break;
    case libhttp::Status::HTTP_VERSION_NOT_SUPPORTED:
      arg[MESSAGE_VARIABLE] = HTTP_VERSION_NOT_SUPPORTED;
      break;
    default:
      break;
  }
  arg[TITLE] = asStr(code) + " | " + arg[MESSAGE_VARIABLE];
  fillTemplate(file, arg);
  arg[SIZE] = asStr(file.size());
  fillTemplate(status, arg);
  std::copy(status.begin(), status.end(), std::back_inserter(*r->buffer));
  std::copy(file.begin(), file.end(), std::back_inserter(*r->buffer));
  return r;
}

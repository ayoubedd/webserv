#include "libhttp/Request.hpp"

std::ostream &operator<<(std::ostream &os, const libhttp::Request &req) {
  os << "method: " << req.method << std::endl;
  os << "path: " << req.path << std::endl;
  os << "version: " << req.version << std::endl;
  os << req.headers << std::endl;
  os << "body: " << std::string(req.body.begin(), req.body.end()) << std::endl;
  return os;
}
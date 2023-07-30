#include "libhttp/Request.hpp"

std::ostream& operator<<(std::ostream &os, const libhttp::Request &req) {
    os << "method: " << req.method << std::endl;
    os << "path: " << req.path << std::endl;
    os << "version: " << req.path << std::endl;
    os << req.headers << std::endl;
    os << "body: " << req.body << std::endl;
    return os;
}
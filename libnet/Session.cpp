#include "libnet/Session.hpp"

libnet::Session::Session(int fd)
    : fd(fd), request(libhttp::Request()), status(READING_HEADERS), reader(fd, this->request) {}

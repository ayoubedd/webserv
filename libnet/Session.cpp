#include "libnet/Session.hpp"

libnet::Session::Session(int fd) : Reader(fd) {
	status = READING_HEADERS;
	this->fd = fd;
}


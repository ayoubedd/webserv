#include "libnet/Session.hpp"

libnet::Session::Session(int fd) : Reader(fd) {
	status = READING_HEADERS;
	this->fd = fd;
}

bool libnet::operator<(libnet::Session const& one, libnet::Session const& two) {
	if (one.fd > two.fd)
		return true;
	else
		return false;
}

bool libnet::operator==(std::pair<const int, libnet::Session> pair, const libnet::Session& session) {
	return pair.first == session.fd ? true : false;
}

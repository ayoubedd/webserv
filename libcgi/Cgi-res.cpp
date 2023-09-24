#include "libcgi/Cgi-res.hpp"
#include "libhttp/constants.hpp"

bool stringPrefixWith(const std::string &str, const std::string pre) {
  for (std::string::size_type i = 0; i < pre.size(); i++) {
    if (str[i] != pre[i])
      return false;
  }
  return true;
}

std::string fromHeaderToStatusLine(const std::string &h) {
  std::string::size_type i, j;
  std::string            statusLine;

  statusLine += "HTTP/1.1 ";
  i = h.find(':') + 1;
  if (h[i] == libhttp::SP)
    i++;
  j = h.find(libhttp::SP, i);
  statusLine += h.substr(i, j) + "\r\n";
  return statusLine;
}

void libcgi::Respons::cgiHeaderToHttpHeader(const std::string &h) {
  const std::string crlf = "\r\n";
  if (stringPrefixWith(h, "Status:")) {
    this->statusLine = fromHeaderToStatusLine(h);
    return;
  }
  std::string::size_type i = h.find(libhttp::SP);
  this->httpHeaders.insert(this->httpHeaders.end(), h.begin(), h.begin() + i);
  this->httpHeaders.insert(this->httpHeaders.end(), h.begin() + i, h.end());
  this->httpHeaders.insert(this->httpHeaders.end(), crlf.begin(), crlf.end());
}

void libcgi::Respons::build() {
  std::string line;
  while (this->cgiHeader.size()) {
    std::vector<char>::size_type i;
    i = 0;
    while (i < this->cgiHeader.size() && this->cgiHeader[i] != libhttp::LF)
      i++;
    line.assign(this->cgiHeader.begin(), this->cgiHeader.begin() + i);
    // std::cerr << '|' << line << '|' << std::endl;
    this->cgiHeaderToHttpHeader(line);
    this->cgiHeader.erase(this->cgiHeader.begin(), this->cgiHeader.begin() + i + 1);
  }
}

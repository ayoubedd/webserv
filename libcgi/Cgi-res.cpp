#include "libcgi/Cgi-res.hpp"
#include "libhttp/constants.hpp"
#include <utility>

libcgi::Respons::Respons()
    : statusLineExists(false){};

bool stringPrefixWith(const std::string &str, const std::string pre) {
  for (std::string::size_type i = 0; i < pre.size(); i++) {
    if (str[i] != pre[i])
      return false;
  }
  return true;
}

std::pair<libcgi::Respons::error, std::string> fromHeaderToStatusLine(const std::string &h) {
  std::string::size_type i, j, k;
  std::string            statusLine;

  statusLine += "HTTP/1.1 ";
  i = h.find(':');
  if (i == std::string::npos)
    return std::make_pair(libcgi::Respons::MALFORMED, "");
  i += 1;
  if (h[i] == libhttp::SP)
    i++;
  j = h.find(libhttp::SP, i);
  if (j == std::string::npos)
    return std::make_pair(libcgi::Respons::MALFORMED, "");
  if (i + 3 != j)
    return std::make_pair(libcgi::Respons::MALFORMED, "");
  statusLine += h.substr(i, j - i);
  k = h.size() - 1;
  if (k - 1 <= j)
    return std::make_pair(libcgi::Respons::MALFORMED, "");
  statusLine += h.substr(j, h.size() - 1) + "\r\n";
  return std::make_pair(libcgi::Respons::OK, statusLine);
}

libcgi::Respons::error libcgi::Respons::cgiHeaderToHttpHeader(const std::string &h) {
  std::vector<char>::size_type  j, k;
  const std::string             crlf = "\r\n";
  std::pair<error, std::string> parsedStatusLine;

  if (stringPrefixWith(h, "Status:")) {
    parsedStatusLine = fromHeaderToStatusLine(h);
    if (parsedStatusLine.first != OK)
      return MALFORMED;
    this->sockBuff.insert(sockBuff.begin(), parsedStatusLine.second.begin(),
                          parsedStatusLine.second.end());
    this->statusLineExists = true;
    return OK;
  }

  std::string::size_type i = h.find(':');
  if (i == std::string::npos)
    return MALFORMED;
  i++;
  j = this->sockBuff.size();
  this->sockBuff.insert(this->sockBuff.end(), h.begin(), h.begin() + i);
  k = this->sockBuff.size();
  if (j + 1 >= k)
    return MALFORMED;
  this->sockBuff.insert(this->sockBuff.end(), h.begin() + i, h.end());
  j = sockBuff.size();
  if (j <= k + 1)
    return MALFORMED;
  this->sockBuff.insert(this->sockBuff.end(), crlf.begin(), crlf.end());
  k = sockBuff.size();
  if (j == k)
    return MALFORMED;
  return OK;
}

libcgi::Respons::error libcgi::Respons::build() {
  std::string line;
  while (this->cgiHeader.size()) {
    std::vector<char>::size_type i;
    i = 0;
    while (i < this->cgiHeader.size() && this->cgiHeader[i] != libhttp::LF)
      i++;
    line.assign(this->cgiHeader.begin(), this->cgiHeader.begin() + i);
    if (this->cgiHeaderToHttpHeader(line) != OK)
      return MALFORMED;
    this->cgiHeader.erase(this->cgiHeader.begin(), this->cgiHeader.begin() + i + 1);
  }
  if (!this->statusLineExists) {
    std::string defaultStatusLine = "HTTP/1.1 200 OK\r\n";
    this->sockBuff.insert(this->sockBuff.begin(), defaultStatusLine.begin(),
                          defaultStatusLine.end());
  }
  return OK;
}

#include "libhttp/Reader.hpp"
#include <algorithm>
#include <cstdio>
#include <sstream>

libhttp::Reader::Reader(int fd, libhttp::Request &r) : fd(fd), current(0), req(r){};

libhttp::Reader::error libhttp::Reader::build() {
  error err;

  if (this->raw.size() == 0) {
    return EMPTY_REQ;
  }
  err = this->buildRequestLine();
  if (err != OK)
    return err;
  err = this->buildRequestHeaders();
  if (err != OK)
    return err;
  err = this->buildRequestBody();
  if (err != OK) {
    return err;
  }
  return OK;
}

/*
  check the return value if it does not have \\r\\n in the end something went wrong
*/
std::string libhttp::Reader::getRequestLineFromRawData() {
  std::string::size_type i;
  std::string res;

  i = 0;
  while (i < this->raw.size() && this->raw[i] != CR) {
    if (this->raw[i + 1] == LF)
      break;
    i++;
  }
  if (this->raw[i] == CR && this->raw[i + 1] == LF)
    i += 2;
  res.assign(this->raw.begin(), this->raw.begin() + i);
  return res;
}

std::string libhttp::Reader::getHeaderstLinesFromRawData() {
  std::string::size_type i, j;
  std::string res;

  i = 0;
  while (i < this->raw.size() && this->raw[i] != CR) {
    if (this->raw[i + 1] == LF)
      break;
    i++;
  }
  if (this->raw[i] == CR && this->raw[i + 1] == LF)
    i += 2;
  if (i >= this->raw.size())
    return "";
  if (raw[i] == CR && raw[i] == LF)
    return "";
  j = i + 1;
  while (j < raw.size()) {
    if (raw[j] == CR && raw[j + 1] == LF && raw[j + 2] == CR && raw[j + 3] == LF)
      break;
    j++;
  }
  if (j >= raw.size())
    return res.assign(this->raw.begin() + i, this->raw.begin() + j);
  return res.assign(this->raw.begin() + i, this->raw.begin() + j + 4);
}

// request-line   = method SP request-target SP HTTP-version
// https://datatracker.ietf.org/doc/html/rfc9112#name-request-line
/*
  if method or path or version is not set probably the sender did not send spaces delimiters
*/
libhttp::Reader::error libhttp::Reader::buildRequestLine() {
  std::string reqline;

  reqline = this->getRequestLineFromRawData();
  if (!reqline.size())
    return REQUEST_LINE_EMPTY;
  if (reqline[reqline.size() - 1] != LF || reqline[reqline.size() - 2] != CR)
    return REQUEST_MISSING_CRLF;
  if (std::count(reqline.begin(), reqline.end(), ' ') != 2) {
    return REQUEST_LINE_WRONG_SP_COUNT;
  }
  std::stringstream s(reqline);
  std::getline(s, req.method, SP);
  std::getline(s, req.path, SP);
  std::getline(s, req.version, CR);
  return OK;
};

libhttp::Reader::error libhttp::Reader::buildRequestHeaders() {
  std::string headline, key, val;
  error err;
  char c;

  err = OK;
  headline = getHeaderstLinesFromRawData();
  if (!headline.size())
    return OK;
  std::stringstream s(headline);
  while (s.peek() != EOF) {
    std::getline(s, key, COLON);
    if (key == std::string("\r\n"))
      break;
    if (s.peek() == SP) {
      s.get(c);
    }
    std::getline(s, val, LF);
    stdStringTrim(val, " \t\r");
    if (req.headers.headers.find(key) != req.headers.headers.end()) {
      err = HEADER_REPEATED;
    }
    req.headers[key] = val;
  }
  return err;
}

libhttp::Reader::error libhttp::Reader::buildRequestBody() {
  std::string::size_type i;

  i = 0;
  while (i < raw.size()) {
    if (raw[i] == CR && raw[i + 1] == LF && raw[i + 2] == CR && raw[i + 3] == LF)
      break;
    i++;
  }
  if (i == raw.size())
    return OK;
  i += 4;
  req.body.assign(raw.begin() + i, raw.end());
  return OK;
}

void stdStringTrimLeft(std::string &str, std::string del) {

  std::string::iterator l;

  l = str.begin();

  while (l < str.end() && del.find(*l) != std::string::npos)
    l++;
  str.erase(str.begin(), l);
}

void stdStringTrimRight(std::string &str, std::string del) {

  std::string::iterator r;

  r = str.end();
  r--;

  while (r > str.begin() && del.find(*r) != std::string::npos)
    r--;
  if (del.find(*r) == std::string::npos)
    r++;
  str.erase(r, str.end());
}

void stdStringTrim(std::string &str, std::string del) {
  stdStringTrimLeft(str, del);
  stdStringTrimRight(str, del);
}

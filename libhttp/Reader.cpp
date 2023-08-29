#include "libhttp/Reader.hpp"
#include "libhttp/constants.hpp"
#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

libhttp::Reader::Reader(int fd, libhttp::Request &req, unsigned int readBuffSize)
    : fd(fd)
    , req(req)
    , readBuffSize(readBuffSize)
    , reqLineEnd(0)
    , headerEnd(0)
    , bodyEnd(0){};

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
  std::string reqLine;

  reqLine.assign(this->raw.begin(), this->raw.begin() + reqLineEnd + 2);
  return reqLine;
}

std::string libhttp::Reader::getHeaderstLinesFromRawData() {
  std::string headers;

  if (this->reqLineEnd + 2 == headerEnd)
    return "";
  headers.assign(this->raw.begin() + reqLineEnd + 2, this->raw.begin() + headerEnd);
  return headers;
}

libhttp::Reader::error libhttp::Reader::buildRequestLine() {
  std::string reqline, reqTarget;

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
  std::getline(s, reqTarget, SP);
  std::getline(s, req.version, CR);
  req.reqTarget.build(reqTarget);
  return OK;
};

libhttp::Reader::error libhttp::Reader::buildRequestHeaders() {
  std::string headline, key, val;
  error       err;
  char        c;

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

std::pair<libhttp::Reader::error, bool> libhttp::Reader::readingRequestHeaderHundler() {
  bool         found;
  unsigned int i;

  found = false;
  for (i = 0; i < raw.size(); i++) {
    if (!this->reqLineEnd && raw[i] == CR && raw[i + 1] == LF) // segfult
      this->reqLineEnd = i;
    if (raw[i] == CR && raw[i + 1] == LF && raw[i + 2] == CR && raw[i + 3] == LF) { // segfult
      found = true;
      break;
    }
  }
  if (!found)
    return std::make_pair(OK, found);
  this->headerEnd = i + 2; // 2 for the /r/n for the last header file
  return std::make_pair(OK, found);
}

std::string getBoundary(std::string &s) {
  std::string            sub;
  std::string::size_type start, end;

  start = s.find("\"", s.find("boundary"));
  end = s.find("\"", start + 1);
  if (start == std::string::npos || start == std::string::npos)
    return "";
  sub = s.substr(start + 1, end - start - 1);
  sub = "--" + sub + "--";
  return sub;
}

/*
 * carefull of passing a buffer smaller than the str
 * */
bool isStrEqualBuff(std::vector<char>::const_iterator it, std::string &str) {
  for (unsigned int i = 0; i < str.size(); i++) {
    if (str[i] != *it)
      return false;
    it++;
  }
  return true;
}

static std::pair<bool, unsigned int> getLastBoundry(const std::vector<char> &data,
                                                    std::string             &boundry) {
  unsigned int i;

  for (i = 0; i <= data.size() - boundry.size(); i++) {
    if (isStrEqualBuff(data.begin() + i, boundry))
      return std::make_pair(true, i);
  }
  return std::make_pair(false, 0);
}

std::pair<libhttp::Reader::error, bool> libhttp::Reader::processChunkedEncoding() {
  std::vector<char>::size_type i;
  std::string                  del = "0\r\n\r\n";
  bool                         found;

  found = false;
  for (i = 0; i < raw.size() - 3; i++) {
    if (isStrEqualBuff(raw.begin() + i, del)) { // possible sigfult
      found = true;
      break;
    }
  }
  if (!found) {
    return std::make_pair(OK, false);
  }
  this->bodyEnd = i;
  return std::make_pair(OK, true);
}

std::pair<libhttp::Reader::error, bool> libhttp::Reader::processContentLength() {
  unsigned int cl = std::atoi(this->req.headers["Content-Length"].c_str()); // handle the error case
  if (!this->bodyEnd) {
    this->bodyEnd = this->headerEnd + cl + 2; // 2 for the /r/n of header delimiter
  }
  if (this->raw.size() - this->headerEnd - 2 >= cl) {
    return std::make_pair(OK, true);
  }
  return std::make_pair(OK, false);
}

std::pair<libhttp::Reader::error, bool> libhttp::Reader::processMultiPartFormData() {
  std::string                   boundry;
  std::pair<bool, unsigned int> bodyEndIdx;

  boundry = getBoundary(this->req.headers["Content-Type"]);
  if (!boundry.size()) {
    return std::make_pair(CANT_FIND_BOUNDRY, false);
  }
  bodyEndIdx = getLastBoundry(this->raw, boundry);
  if (!bodyEndIdx.first) {
    return std::make_pair(OK, false);
  }
  this->bodyEnd = bodyEndIdx.second + boundry.size();
  return std::make_pair(OK, true);
}

std::pair<libhttp::Reader::error, bool > libhttp::Reader::readingBodyHundler() {
  if (this->req.headers.headers.find("Transfer-Encoding") != this->req.headers.headers.end()) {
    return processChunkedEncoding();
  }
  if (this->req.headers.headers.find("Content-Length") != this->req.headers.headers.end()) {
    return processContentLength();
  }
  if (this->req.headers.headers.find("Content-Type") != this->req.headers.headers.end()) {
    return processMultiPartFormData();
  }
  return std::make_pair(OK, true); // there is no body
}

std::pair<libhttp::Reader::error, libnet::SessionState>
libhttp::Reader::processReadBuffer(libnet::SessionState state) {
  std::pair<error, bool> complete;
  error                  err;

  if (state == libnet::READING_HEADERS) {
    err = OK;
    complete = readingRequestHeaderHundler();
    if (complete.first != OK) {
      return std::make_pair(complete.first, state);
    }
    if (!complete.second) {
      return std::make_pair(OK, libnet::READING_HEADERS);
    }
    err = buildRequestLine(); // this need to be moved
    if (err != OK) {
      return std::make_pair(err, state);
    }
    err = buildRequestHeaders(); // this need to be moved
    if (err != OK) {
      return std::make_pair(err, state);
    }
  }
  complete = readingBodyHundler();
  if (complete.first != OK) {
    return std::make_pair(complete.first, state);
  }
  if (!complete.second) {
    return std::make_pair(OK, libnet::READING_BODY);
  }
  err = buildRequestBody();
  return std::make_pair(err, libnet::READING_FIN);
}

std::pair<libhttp::Reader::error, libnet::SessionState>
libhttp::Reader::read(libnet::SessionState state) {
  char                                   buff[this->readBuffSize];
  ssize_t                                buffLen;
  std::pair<error, libnet::SessionState> futureState;

  buffLen = recv(this->fd, buff, readBuffSize, 0);
  // buffLen = ::read(this->fd, buff, readBuffSize);

  if (buffLen < 0) {
    return std::make_pair(REQUEST_READ_FAILED, state);
  }
  if (!buffLen)
    return std::make_pair(OK, libnet::READING_FIN); // socket closed

  this->raw.insert(this->raw.end(), buff, buff + buffLen);
  futureState = processReadBuffer(state);
  return futureState;
}

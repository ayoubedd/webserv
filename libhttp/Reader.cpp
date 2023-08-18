#include "libhttp/Reader.hpp"
#include "libhttp/constants.hpp"
#include <algorithm>
#include <climits>
#include <cstdio>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

libhttp::Reader::Reader(int fd, libhttp::Request &req, unsigned int readBuffSize)
    : fd(fd), req(req), readBuffSize(readBuffSize), reqLineEnd(0), headerEnd(0), bodyEnd(0){};

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

std::pair<libhttp::Reader::error, bool>
libhttp::Reader::readingRequestHeaderHundler(const char *buff, unsigned int len) {
  bool found;
  unsigned int i;

  found = false;
  for (i = 0; i < len - 3; i++) {
    if (!this->reqLineEnd && buff[i] == libhttp::CR && buff[i + 1] == LF)
      this->reqLineEnd = this->raw.size() - len + i;
    if (buff[i] == libhttp::CR && buff[i + 1] == LF && buff[i + 2] == CR && buff[i + 3] == LF) {
      found = true;
      break;
    }
  }
  if (!found)
    return std::make_pair(OK, found);
  this->headerEnd = this->raw.size() - len + i + 2; // 2 for the /r/n for the last header file
  // this->raw.insert(raw.end(), buff, buff + len);
  return std::make_pair(OK, found);
}

std::string getBoundary(std::string &s) {
  std::string sub;
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
bool isStrEqualBuff(const char *buff, std::string &str) {
  for (unsigned int i = 0; i < str.size(); i++) {
    if (str[i] != buff[i])
      return false;
  }
  return true;
}

static unsigned int getLastBoundry(const char *buff, unsigned int len, std::string &boundry) {
  unsigned int i;

  for (i = 0; i < len - boundry.size(); i++) {
    if (isStrEqualBuff(&buff[i], boundry))
      return i;
  }
  return UINT_MAX; // this may collaps on large body values
}

std::pair<libhttp::Reader::error, bool> libhttp::Reader::processChunkedEncoding(const char *buff,
                                                                                unsigned int len) {
  unsigned int i;
  bool found;
  std::string del = "0\r\n\r\n";
  found = false;
  for (i = 0; i < len - 3; i++) {
    if (isStrEqualBuff(&buff[i], del)) {
      found = true;
      break;
    }
  }
  if (!found) {
    return std::make_pair(OK, false);
  }
  this->bodyEnd = this->raw.size() - len + i;
  return std::make_pair(OK, true);
}

std::pair<libhttp::Reader::error, bool> libhttp::Reader::processContentLength() {
  unsigned int cl = std::atoi(this->req.headers["Content-Length"].c_str()); // handle the error case
  if (!this->bodyEnd) {
    this->bodyEnd = this->headerEnd + cl + 2; // 2 for the /r/n of header delimiter
  }
  if (this->raw.size() - this->headerEnd - 2 > cl) {
    return std::make_pair(OK, true);
  }
  return std::make_pair(OK, false);
}

std::pair<libhttp::Reader::error, bool>
libhttp::Reader::processMultiPartFormData(const char *buff, unsigned int len) {

  std::string boundry;
  unsigned int bodyEndIdx;

  boundry = getBoundary(this->req.headers["Content-Type"]);
  if (!boundry.size())
    return std::make_pair(CANT_FIND_BOUNDRY, false);
  bodyEndIdx = getLastBoundry(buff, len, boundry);
  this->bodyEnd = bodyEnd == UINT_MAX ? this->bodyEnd : this->raw.size() - len + bodyEndIdx;
  if (bodyEnd == UINT_MAX) {
    return std::make_pair(OK, false);
  }
  return std::make_pair(OK, true);
}

std::pair<libhttp::Reader::error, bool > libhttp::Reader::readingBodyHundler(const char *buff,
                                                                             unsigned int len) {
  if (this->req.headers.headers.find("Transfer-Encoding") != this->req.headers.headers.end()) {
    return processChunkedEncoding(buff, len);
  }
  if (this->req.headers.headers.find("Content-Length") != this->req.headers.headers.end()) {
    return processContentLength();
  }
  if (this->req.headers.headers.find("Content-Type") != this->req.headers.headers.end()) {
    return processMultiPartFormData(buff, len);
  }
  return std::make_pair(OK, true); // there is no body
}

std::pair<libhttp::Reader::error, libnet::SessionState>
libhttp::Reader::processReadBuffer(libnet::SessionState state, const char *buff, ssize_t buffLen) {
  std::pair<error, bool> complete;
  error err;

  if (state == libnet::READING_HEADERS) {
    err = OK;
    complete = readingRequestHeaderHundler(buff, buffLen);
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
  if (state == libnet::READING_HEADERS || state == libnet::READING_BODY) {
    complete = readingBodyHundler(buff, buffLen);
    if (complete.first != OK) {
      return std::make_pair(complete.first, state);
    }
    if (!complete.second) {
      return std::make_pair(OK, libnet::READING_BODY);
    }
    err = buildRequestBody();
    // err
    return std::make_pair(err, libnet::READING_FIN);
  }
  return std::make_pair(OK, state); // this should be fix code should not arrive here
}

std::pair<libhttp::Reader::error, libnet::SessionState>
libhttp::Reader::read(libnet::SessionState state) {
  char buff[this->readBuffSize];
  ssize_t buffLen;
  std::pair<error, libnet::SessionState> futureState;

  // size = recv(this->fd, buff, readBuffSize, 0);
  buffLen = ::read(this->fd, buff, readBuffSize);

  if (buffLen < 0) {
    return std::make_pair(REQUEST_READ_FAILED, state);
  }
  if (!buffLen)
    return std::make_pair(OK, libnet::READING_FIN); // socket closed

  this->raw.insert(this->raw.end(), buff, buff + buffLen);
  futureState = processReadBuffer(state, buff, buffLen);
  return futureState;
}

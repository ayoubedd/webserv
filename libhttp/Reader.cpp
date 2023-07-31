#include "libhttp/Reader.hpp"

libhttp::Reader::Reader(int fd) : fd(fd), current(0){};

libhttp::Reader::error libhttp::Reader::build() {
  if (this->raw.size() == 0) {
    return EMPTY_REQ;
  }
  error err = buildRequestLine();
  if (err != OK) {
    return err;
  }
  return OK;
}

libhttp::Reader::error libhttp::Reader::buildRequestLine() {
  std::vector<char>::size_type i, j;
  while (raw[current] && raw[current] != SP)
    current++;
  if (raw[current] != SP) {
    return WRONG_TOK_AFTER_METHOD;
  }
  this->req.method.assign(this->raw.begin(), this->raw.begin() + current);

  i = ++current;
  while (raw[current] && raw[current] != SP)
    current++;
  if (i == current) {
    return MISSING_URI;
  }
  if (raw[current] != SP) {
    return WRONG_TOK_AFTER_URI;
  }
  this->req.path.assign(this->raw.begin() + i, this->raw.begin() + current);
  j = ++current;
  while (raw[current] && raw[current + 1] && raw[current] != CR && raw[current + 1] != LF)
    current++;
  if (j == current) {
    return MISSING_HTTP_V;
  }
  if (raw[current] != CR || raw[current + 1] != LF) {
    return MISSING_HTTP_V;
  }
  this->req.version.assign(this->raw.begin() + j, this->raw.begin() + current);
  current += 2;
  return OK;
}
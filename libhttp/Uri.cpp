#include "libhttp/Uri.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>

// const char *libhttp::Uri::enc[] = {
//     "%", " ", "!", "\"", "#", "$", "&", "'", "(", ")", "*",
//     "+", ",", "/", ":",  ";", "=", "?", "@", "[", "]",
// };

const char libhttp::Uri::ENC[] = {
    '%', ' ', '!', '"', '#', '$', '&', '\'', '(', ')', '*',
    '+', ',', '/', ':', ';', '=', '?', '@',  '[', ']',
};
const std::string libhttp::Uri::DEC[] = {
    "%25", "%20", "%21", "%22", "%23", "%24", "%26", "%27", "%28", "%29", "%2A",
    "%2B", "%2C", "%2F", "%3A", "%3B", "%3D", "%3F", "%40", "%5B", "%5D",
};

static std::string::size_type searchInEnc(char c) {
  std::string::size_type i;

  for (i = 0; i < libhttp::Uri::LEN; i++) {
    if (libhttp::Uri::ENC[i] == c)
      return i;
  }
  return std::string::npos;
}

std::string libhttp::Uri::encode(const std::string &uri) {
  std::string ss;
  std::string::size_type i, idx;

  for (i = 0; i < uri.size(); i++) {
    idx = searchInEnc(uri[i]);
    if (idx == std::string::npos) {
      ss += uri[i];
      continue;
    }
    ss += DEC[idx];
  }
  return ss;
}

std::string libhttp::Uri::decode(const std::string &uri) {
  std::string ss;
  std::string::size_type i;
  const std::string *str;

  for (i = 0; i < uri.size(); i++) {
    i + 3 <= uri.size() ? str = std::find(DEC, DEC + LEN, uri.substr(i, 3)) : str = DEC + LEN;

    if (str == DEC + LEN) {
      ss += uri[i];
      continue;
    }
    ss += ENC[str - DEC];
    i += 2;
  }
  return ss;
}

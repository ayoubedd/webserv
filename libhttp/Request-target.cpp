#include "libhttp/Request-target.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

const char libhttp::RequestTarget::ENC[] = {
    '%', ' ', '!', '"', '#', '$', '&', '\'', '(', ')', '*',
    '+', ',', '/', ':', ';', '=', '?', '@',  '[', ']',
};
const std::string libhttp::RequestTarget::DEC[] = {
    "%25", "%20", "%21", "%22", "%23", "%24", "%26", "%27", "%28", "%29", "%2A",
    "%2B", "%2C", "%2F", "%3A", "%3B", "%3D", "%3F", "%40", "%5B", "%5D",
};

libhttp::RequestTarget::RequestTarget()
    : path("")
    , params()
    , anchor()
    , rawPramas() {}

static std::string::size_type searchInEnc(char c) {
  std::string::size_type i;

  for (i = 0; i < libhttp::RequestTarget::LEN; i++) {
    if (libhttp::RequestTarget::ENC[i] == c)
      return i;
  }
  return std::string::npos;
}

std::string libhttp::RequestTarget::encode(const std::string &uri) {
  std::string            ss;
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

std::string libhttp::RequestTarget::decode(const std::string &uri) {
  std::string            ss;
  std::string::size_type i;
  const std::string     *str;

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

static std::string getPathFromReqTarget(std::string &reqTarget) {
  std::string::size_type i;

  i = 0;
  while (i < reqTarget.size() && reqTarget[i] != '?' && reqTarget[i] != '#')
    i++;
  return reqTarget.substr(0, i);
}

static void insertKeyAndValToParams(std::string                         keyAndVal,
                                    std::map<std::string, std::string> &params) {
  std::stringstream ss(keyAndVal);
  std::string       key, val;
  std::getline(ss, key, '=');
  std::getline(ss, val);
  params.insert(std::make_pair(key, val));
}

static void getParamsFromReqTarget(std::string                        &reqTarget,
                                   std::map<std::string, std::string> &params,
                                   std::string                        &rawPramas) {
  std::string::size_type i, j;
  std::string            keyAndValAsStr;

  i = 0;
  while (i < reqTarget.size() && reqTarget[i] != '?')
    i++;
  if (i == reqTarget.size())
    return;
  i += 1;
  j = i;
  while (j < reqTarget.size() && reqTarget[j] != '#')
    j++;
  if (i + 1 == j)
    return;
  rawPramas = reqTarget.substr(i, j - i);
  std::stringstream paramsAsStr(rawPramas);
  while (std::getline(paramsAsStr, keyAndValAsStr, '&')) {
    insertKeyAndValToParams(keyAndValAsStr, params);
  }
}

static std::string getAnchoreFromReqTrget(std::string &reqTarget) {

  std::string::size_type i, j;

  i = 0;
  while (i < reqTarget.size() && reqTarget[i] != '#')
    i++;
  if (i == reqTarget.size())
    return "";
  i += 1;
  j = i;
  while (j < reqTarget.size() && reqTarget[j] != '?')
    j++;
  if (i + 1 == j)
    return "";
  return reqTarget.substr(i, j);
}

void libhttp::RequestTarget::build(std::string &reqTarget) {
  reqTarget = this->decode(reqTarget);
  this->path = getPathFromReqTarget(reqTarget);
  getParamsFromReqTarget(reqTarget, this->params, this->rawPramas);
  this->anchor = getAnchoreFromReqTrget(reqTarget);
}

std::string libhttp::RequestTarget::getPathFromUrl(const std::string &url) {
  std::string            path;
  std::string::size_type i, j;

  path = url;
  if (url.front() == '/')
    return path;
  i = path.find('/');
  if (i == std::string::npos)
    return "";
  i = path.find('/', i + 1);
  if (i == std::string::npos)
    return "";
  i = path.find('/', i + 1);
  if (i == std::string::npos)
    return "";
  j = path.rfind('/');
  return path.substr(i, j - i + 1);
}

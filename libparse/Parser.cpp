#include "Config.hpp"
#include "utilities.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <utility>
// #include <unistd.h>

static bool strStartWith(const std::string str, const std::string prefix) {
  std::string::size_type i;

  for (i = 0; i < prefix.size(); i++) {
    if (prefix[i] != str[i])
      return false;
  }
  return true;
}

libparse::Domain *getDefeaultSever(libparse::Domains &domains, std::string server) {
  for (libparse::Domains::iterator it = domains.begin(); it != domains.end(); it++) {
    if (strStartWith(it->first, server))
      return &it->second;
  };
  return NULL;
}

std::pair<bool, std::string> libparse::parser(libparse::Config              &config,
                                              std::vector<libparse::tokens> &tokens) {
  bool is = false;
  if (tokens.size() <= 1)
    return std::make_pair(false, "Config Is empty ");
  size_t                       i = 0;
  std::pair<bool, std::string> res;
  while ((tokens[i].lexeme == "log_error" || tokens[i].lexeme == "log_info") && i < 4 &&
         i < tokens.size()) {
    res = setUpLog(config, tokens, &i);
    if (!res.first)
      return res;
    i++;
  }
  while (tokens[i].type != libparse::tokens::ENDFILE && i < tokens.size() - 1) {
    if (tokens[i].lexeme == "default") {
      if (is)
        return std::make_pair(false, "You must one default Server :) ");
      res = setUpDefaultSever(config, tokens, &i);
      if (!res.first)
        return res;
      is = true;
      libparse::Domains::iterator it = config.domains.find(res.second);
      if (it != config.domains.end())
        config.defaultServer = &it->second;
    } else {
      res = SetUpServer(config, tokens, &i);
      if (!res.first)
        return res;
    }
    continue;
  }
  return std::make_pair(true, "");
}

std::pair<bool, std::string> checkDuplicatePort(libparse::Config &config) {
  libparse::Domains::iterator itD;
  libparse::Domains::iterator it;
  itD = config.domains.begin();
  std::string port;
  while (itD != config.domains.end()) {
    port = itD->second.port;
    it = ++itD;
    while (it != config.domains.end()) {
      if (it->second.port == port)
        return std::make_pair(false, it->first);
      it++;
    }
  }
  return std::make_pair(true, "");
}

std::pair<bool, std::string> checkDefaultRout(libparse::Config &config) {
  libparse::Domains::iterator itD;
  libparse::Routes::iterator  itR;

  itD = config.domains.begin();
  while (itD != config.domains.end()) {
    itR = itD->second.routes.find("/");
    if (itR == itD->second.routes.end())
      return std::make_pair(false, itD->first);
    if (itR->second.index.empty())
      return std::make_pair(false, "index is empty in the server :" + itD->first);
    if (itR->second.root.empty())
      return std::make_pair(false, "root is empty in the server :" + itD->first);
    itD++;
  }
  return std::make_pair(true, "");
}

bool libparse::checkConfig(const std::string &fileName, libparse::Config &config) {
  std::string                   contentFile;
  std::vector<libparse::tokens> tokens;
  std::pair<bool, std::string>  res;

  contentFile = libparse::readFile(fileName);
  if (contentFile.empty())
    return false;
  libparse::lexer(tokens, contentFile);
  res = parser(config, tokens);
  if (!res.first) {
    std::cout << "Error invalide Token : " << res.second << std::endl;
    return res.first;
  }
  res = checkFileExist(config);
  if (!res.first) {
    std::cout << "File Not Exist or Not Editable : " << res.second << std::endl;
    return res.first;
  }
  res = checkDefaultRout(config);
  if (!res.first) {
    std::cout << "Error Default Route " << res.second << std::endl;
    return res.first;
  }
  res = checkDuplicatePort(config);
  if (!res.first) {
    std::cout << "Error Mulitple Port " << res.second << std::endl;
    return res.first;
  }
  return true;
}

bool libparse::Config::init() {
  int fdInfo, fdError;

  if (!this->log_info.empty()) {
    fdInfo = open(this->log_info.c_str(), O_WRONLY);
    if (fdInfo < 0)
      return false;
    if (::dup2(fdInfo, 1) < 0)
      return false;
    if (close(fdInfo) < 0)
      return false;
  }
  if (!this->log_error.empty()) {

    fdError = open(this->log_info.c_str(), O_WRONLY);
    if (fdError < 0)
      return false;
    if (dup2(fdError, 2) < 0)
      return false;
    if (close(fdError) < 0)
      return false;
  }
  return true;
}

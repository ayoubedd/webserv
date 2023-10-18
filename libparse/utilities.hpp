#pragma once

#include "Config.hpp"
#include <cstddef>
#include <string>

int                          convertToInt(std::string str);
bool                         convertStrToBool(std::string str);
std::pair<bool, std::string> checkFileExist(libparse::Config &config);
std::pair<bool, std::string> setUpLog(libparse::Config              &config,
                                      std::vector<libparse::tokens> &tokens, size_t *i);

bool checkIsKeyServer(libparse::token::t_type type);

bool checkIsKeyRoute(libparse::token::t_type type);

bool checkIsKeyRouteStr(std::string key);

void skipWithSpace(std::string &content, size_t *i);

std::string getWord(std::string &content, size_t *i);

bool isWhiteSpace(char c);

bool checkMethod(std::string method);

std::pair<bool, std::string> setUpMethods(libparse::Routes &route, std::string &nameRoute,
                                          std::vector<libparse::tokens> &tokens, size_t *i);

std::pair<bool, std::string> setUpToken(libparse::Routes &route, std::string &nameRoute,
                                        std::vector<libparse::tokens> &tokens, size_t *i);
std::pair<bool, std::string> setUpCgi(libparse::Routes &route, std::string &nameRoute,
                                      std::vector<libparse::tokens> &tokens, size_t *i);

std::pair<bool, std::string> setUpRout(libparse::Config &config, std::string &nameDomain,
                                       std::vector<libparse::tokens> &tokens, size_t *i);

std::pair<bool, std::string> setUpKey(libparse::Config &config, std::string nameDomain,
                                      std::vector<libparse::tokens> &tokens, size_t *i);

std::pair<bool, std::string> SetUpServer(libparse::Config              &config,
                                         std::vector<libparse::tokens> &tokens, size_t *i);

std::pair<bool, std::string> setUpDefaultSever(libparse::Config              &config,
                                               std::vector<libparse::tokens> &tokens, size_t *i);

std::pair<bool, std::string> checkDuplicatePort(libparse::Config &config);

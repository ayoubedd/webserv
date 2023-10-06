#pragma once

#include "Config.hpp"

std::vector<std::string > split(const std::string input);
void check(std::string &str);
libparse::token::t_type getTypeFromString(std::string typeStr);
void throw_error(std::string token, std::string msg);
void skipEndLine(std::vector<std::string> content, size_t &i);
bool checkDomain(std::string domain);
void setNewToken(libparse::token::t_type type, std::string lexeme,
                 std::vector<libparse::tokens> &token);
bool checkValueOfMethode(std::string value);
bool checkValidKey(std::string key);
bool checkValidKeyOfRoute(std::string key);
int methodes(std::vector<libparse::tokens> &tokens, std::vector<std::string> content, size_t &i);
int checkValue(std::string value);
int consumeToken(std::vector<libparse::tokens> &tokens, std::vector<std::string> &content,
                 size_t &i);
int domain(std::vector<libparse::tokens> &tokens, std::string domain);
int configRout(std::vector<libparse::tokens> &tokens, std::vector<std::string> &content, size_t &i);
int configOutRout(std::vector<libparse::tokens> &tokens, std::vector<std::string> &content,
                  size_t &i);
int config(std::vector<libparse::tokens> &tokens, std::vector<std::string> &content, size_t &i);
void cleanUp(std::vector<libparse::tokens> &tokens);
bool convertStrToBool(std::string str);
bool CheckValueOfOutRout(libparse::token::t_type type);
bool CheckValueOfInRout(libparse::token::t_type type);
void setDefautlValue(libparse::Domain &domain, libparse::RouteProps &routeProps);
std::string SetDomain(std::vector<libparse::tokens> &tokens, libparse::Domain &domain);
void SetRoute(std::vector<libparse::tokens> &tokens, libparse::Domain &domain);
void SetValue(std::vector<libparse::tokens> &tokens, libparse::RouteProps &routeProps);
void SetConfigInRoute(std::vector<libparse::tokens> &tokens, libparse::RouteProps &routeProps,
                      libparse::Domain &domain);
libparse::Domains setTokenInStruct(std::vector<libparse::tokens> &tokens);
bool checkDefaulfServer(std::vector<std::string> content,size_t &i);

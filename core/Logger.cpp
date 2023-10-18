#include "core/Logger.hpp"
#include "../libparse/Config.hpp"
#include "libhttp/Request.hpp"
#include "libhttp/Response.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <utility>

const char *Webserv::logger::LOGGERLINE = "{IPv4} {METHODS} {PATH} {VERSIONHTTP}";
const char *Webserv::logger::IPV4 = "IPv4";
const char *Webserv::logger::METHOD = "METHOD";
const char *Webserv::logger::VERSIONHTTP = "VERSIONHTTP";
const char *Webserv::logger::PATH = "PATH";

Webserv::logger::error Webserv::logger::log(libparse::Config &config) {
  std::string helo;
  if (config.)

    return logger::error::OK;
}

void Webserv::logger::loggerInfo(libhttp::Request &request) {

  std::string tmp;
  // std::string loggerline;
  // std::map<std::string, std::string> map;
  // char                               ip4[INET_ADDRSTRLEN] = {0};

  // inet_ntop(AF_INET, &request.clientAddr->sin_addr, ip4, INET_ADDRSTRLEN);
  // map.insert(std::make_pair(IPV4, ip4));
  // map.insert(std::make_pair(METHOD, request.method));
  // map.insert(std::make_pair(PATH, request.reqTarget.path));
  // map.insert(std::make_pair(METHOD, "HTTP/1.1"));
  // libhttp::ErrorGenerator::fillTemplate(loggerline, map);
  // std::cout << loggerline << std::endl;
}

void Webserv::logger::loggerError(libhttp::Request &request) {

  // std::string                        loggerline(LOGGERLINE);
  // std::map<std::string, std::string> map;
  // char                               ip4[INET_ADDRSTRLEN] = {0};

  // inet_ntop(AF_INET, &request.clientAddr->sin_addr, ip4, INET_ADDRSTRLEN);
  // map.insert(std::make_pair(IPV4, ip4));
  // map.insert(std::make_pair(METHOD, request.method));
  // map.insert(std::make_pair(PATH, request.reqTarget.path));
  // map.insert(std::make_pair(METHOD, "HTTP/1.1"));
  // libhttp::ErrorGenerator::fillTemplate(loggerline, map);
  // std::cerr << loggerline << std::endl;
}

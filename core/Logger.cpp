#include "core/Logger.hpp"
#include "../libparse/Config.hpp"
#include "libhttp/Request.hpp"
#include "libhttp/Response.hpp"
#include <arpa/inet.h>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <utility>

const char *Webserv::Logger::LOGGERLINE = " {TIME} {IPv4} {METHOD} {PATH} {VERSIONHTTP}";
const char *Webserv::Logger::IPV4 = "{IPv4}";
const char *Webserv::Logger::METHOD = "{METHOD}";
const char *Webserv::Logger::VERSIONHTTP = "{VERSIONHTTP}";
const char *Webserv::Logger::PATH = "{PATH}";
const char *Webserv::Logger::TIME = "{TIME}";

std::string getCurrentTime() {
  std::time_t rawTime;
  std::tm    *timeInfo;
  char        timeString[80];

  std::time(&rawTime);
  timeInfo = std::localtime(&rawTime);

  std::strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", timeInfo);

  return std::string(timeString);
}

void Webserv::Logger::log(const libhttp::Request &request, int option) {

  std::string                        tmp;
  std::string                        loggerline(LOGGERLINE);
  std::map<std::string, std::string> map;
  char                               ip4[INET_ADDRSTRLEN] = {0};

  inet_ntop(AF_INET, &request.clientAddr->sin_addr, ip4, INET_ADDRSTRLEN);
  map.insert(std::make_pair(TIME, getCurrentTime()));
  map.insert(std::make_pair(IPV4, ip4));
  map.insert(std::make_pair(METHOD, request.method));
  map.insert(std::make_pair(PATH, request.reqTarget.path));
  map.insert(std::make_pair(VERSIONHTTP, "HTTP/1.1"));
  libhttp::ErrorGenerator::fillTemplate(loggerline, map);

  if (option & INFO)
    std::cout << loggerline << std::endl;
  if (option & ERROR)
    std::cerr << loggerline << std::endl;
}

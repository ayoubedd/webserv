#include "libparse/match.hpp"
#include "libparse/Config.hpp"
#include "libparse/TestParser.hpp"
#include <algorithm>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

int main(int argc, char *argv[]) {
  libparse::Config config;

  if (argc < 2) {
    std::cerr << "error: missing config file \n";
    return 0;
  }
  std::string filename = argv[1];
  bool res = libparse::checkConfig(filename,config);
  if(res)
    std::cout << "-----------> Config Is True-------------------------- " << std::endl;
  else
    std::cout << "-----------> Config Is Flase-------------------- " << std::endl;
 printConfig(config);


  return 0;
}

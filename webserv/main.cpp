#include "libparse/Config.hpp"
#include "libparse/TestParser.hpp"
#include "libparse/utilities.hpp"

int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::cout << "error: Missing config file \n";
    return 0;
  }

  libparse::Domains map;
  std::vector<libparse::tokens> tokens;

  libparse::parser(argv[1], map, tokens);
  printConfig(map);
  return 0;
}

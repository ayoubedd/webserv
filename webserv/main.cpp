#include "libhttp/Headers.hpp"
#include "libhttp/Reader.hpp"

int main(int argc, char *argv[]) {
  if (!libhttp::TestReaderBuildRequestLine()) {
    std::cout << "Failed" << std::endl;
  }
  return 0;
}

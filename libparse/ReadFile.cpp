#include "Config.hpp"
#include <algorithm>

void ft_replace(std::string &str, const std::string &old_value, const std::string &new_value) {
  size_t pos = 0;
  while ((pos = str.find(old_value, pos)) != std::string::npos) {
    str.replace(pos, old_value.length(), new_value);
    pos += new_value.length();
  }
}

std::string libparse::readFile(std::string filename) {
  std::ifstream inputFile(filename.c_str());
  std::stringstream buffer;
  std::string fileContent;

  if (!inputFile.is_open()) {
    std::cerr << "Error opening file: " << filename << std::endl;
    return "";
  }
  buffer << inputFile.rdbuf();
  inputFile.close();
  fileContent = buffer.str();

  ft_replace(fileContent, "\n\n", "\n");
  ft_replace(fileContent, "\n", " endline ");
  return fileContent;
}

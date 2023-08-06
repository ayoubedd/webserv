#include "Config.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

std::string readFile(std::string filename)
{ 
  std::ifstream inputFile(filename);
  std::stringstream buffer;
  std::string fileContent;

  if (!inputFile.is_open()) 
  {
      std::cout << "Error opening file: " << filename << std::endl;
        return "";
  }
  buffer << inputFile.rdbuf(); 
  inputFile.close();
  fileContent = buffer.str();
  std::replace(fileContent.begin(), fileContent.end(), '\n', ' ');
  return fileContent;
}

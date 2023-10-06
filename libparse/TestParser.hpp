#pragma once
#include "Config.hpp"
std::string getTypeFromInt(int type);
void        printVector(std::vector<std::string> v);
void        printVectorToken(std::vector<libparse::tokens> v);
void        printConfig(libparse::Config config);

#pragma once
#include "../libparse/Config.hpp"

bool fileExists(std::string &filename);
bool isFolder(std::string &path);
bool findResource(std::string &path);
bool deleteDirectory(const char* path);

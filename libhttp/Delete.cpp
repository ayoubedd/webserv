#include "Delete.hpp"
#include "utilities.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

bool deleteDirectory(const char* path) {
    struct dirent* entry;
    DIR* dir = opendir(path);

    if (dir == nullptr) {
        return false;
    }
    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            std::string entryPath = std::string(path) + "/" + entry->d_name;
            struct stat statBuf;

            if (stat(entryPath.c_str(), &statBuf) == 0) {
                if (S_ISDIR(statBuf.st_mode)) {
                    deleteDirectory(entryPath.c_str());
                }
                else {
                    if (remove(entryPath.c_str()) != 0) {
                        std::cout << "in while \n";
                        return false;
                    }
                }
            }
        }
    }
    closedir(dir);
    if (remove(path) != 0) {
        std::cout << "in the last\n";
       return false;
   }
}

libhttp::Delete::t_error libhttp::Deletes(std::string &path)
{
    std::cout << path <<std::endl;
    if(!findResource(path))
    {
        std::cout << "not found \n";
        return libhttp::Delete::FILE_NOT_FOUND;
    }
    if(isFolder(path))
    {
        std::cout << "dir \n"<<std::endl;
        if (deleteDirectory(path.c_str()))
            return libhttp::Delete::OK;
        else
        {
            std::cout << "error \n";
            return libhttp::Delete::FORBIDDEN;
        }
    }
    else
    {
        if (remove(path.c_str()) != 0) {
            return libhttp::Delete::FORBIDDEN;
        }
        else
            libhttp::Delete::OK;
    }
}

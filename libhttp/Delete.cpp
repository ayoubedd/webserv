#include "Delete.hpp"
#include "utilities.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

libhttp::Delete::t_error libhttp::Deletes(std::string &path)
{
    if(!findResource(path))
    {
        return libhttp::Delete::FILE_NOT_FOUND;
    }
    if(isFolder(path))
    {
        if (deleteDirectory(path.c_str()))
            return libhttp::Delete::OK;
        else
        {
            return libhttp::Delete::FORBIDDEN;
        }
    }
    else
    {
        if (remove(path.c_str()) != 0) {
            return libhttp::Delete::FORBIDDEN;
        }
        else
            return libhttp::Delete::OK;
    }
    return libhttp::Delete::FILE_NOT_FOUND;
}

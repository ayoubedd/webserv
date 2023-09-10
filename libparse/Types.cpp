#include "libparse/Types.hpp"

std::map<std::string, std::string> libparse::Types(void)
{
    std::ifstream file("conf/mime.types");
    std::map<std::string, std::string> types;

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if(line == "\n")
                break;
            types.insert({line.substr(0,line.find("=")), line.substr(line.rfind("=")+1, line.length() -1)});
        }
    file.close();
    }
    return types;
}

std::string libparse::getTypeFile(std::map<std::string, std::string> types, std::string file)
{
    std::string type;
    type = file.substr(file.rfind(".") + 1, file.size() - file.rfind("."));
    std::map<std::string, std::string>::iterator it;
    it = types.begin();

    for (it = types.begin(); it != types.end(); ++it)
    {
        if (it->second == type)
            return it->first;
    }
    return  "text/plain";
}
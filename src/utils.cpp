#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>
std::vector<std::string> split(const std::string &str, char delim)
{
    std::vector<std::string> tokens;
    std::stringstream mySstream(str);
    std::string temp;

    while (getline(mySstream, temp, delim))
    {
        tokens.push_back(temp);
    }

    return tokens;
}
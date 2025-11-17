#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <iostream>

#ifndef UTILS
#define UTILS
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
void print_progress(int done, int total) {
    int width = 30;
    int pos = (done * width) / total;
    std::cout << "[";
    for (int i = 0; i < width; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << done << "/" << total << "\r" << std::flush;
}

#endif
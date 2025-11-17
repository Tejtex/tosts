#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace std::chrono;

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define CURSOR2 "\033[2F"



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
void print_progress(int done, int total, milliseconds dur, Stats &stats) {
    int width = 30;
    int pos = (done * width) / total;
    std::cout << CYAN << "[";
    for (int i = 0; i < width; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << done << "/" << total << "\n" << std::flush;

    auto secs = duration_cast<seconds>(dur);
    auto h = duration_cast<hours>(secs);
    auto m = duration_cast<minutes>(secs - h);
    auto s = secs - h - m;

    std::cout << CYAN << '['
        << std::setw(2) << std::setfill('0') << h.count() << ':'
        << std::setw(2) << std::setfill('0') << m.count() << ':'
        << std::setw(2) << std::setfill('0') << s.count()
        << "] " << std::flush;
    std::cout << GREEN << stats.ok.size() << " correct; ";
    std::cout << RED << stats.mle.size() + stats.re.size() + stats.tle.size() + stats.wa.size() << " wrong\n" << RESET << CURSOR2 << std::flush;
}

#endif
#include <vector>
#include <string>

#ifndef STATS
#define STATS
struct Stats
{
    std::vector<std::string> skipped;
    std::vector<std::string> ok;
    std::vector<std::string> wa;
    std::vector<std::string> re;
};
#endif
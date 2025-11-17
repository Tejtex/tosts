#include <vector>
#include <string>

#ifndef STATS
#define STATS
struct Stats
{
    long long sum_time_ok;
    long long max_time_ok;
    std::vector<std::string> skipped;
    std::vector<std::string> ok;
    std::vector<std::string> wa;
    std::vector<std::string> tle;
    std::vector<std::string> mle;
    std::vector<std::string> re;
};
#endif
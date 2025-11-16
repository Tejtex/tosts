#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "load_tests.cpp"
#include "runner.cpp"
#include "utils.cpp"

struct Args
{
    std::string in, out;
    std::vector<std::string> cmd;
};

bool parse(int argc, char *argv[], Args &a)
{
    std::vector<std::string> v(argv + 1, argv + argc);
    bool has_i = false, has_o = false;
    size_t i = 0;
    while (i < v.size())
    {
        if (v[i] == "-i")
        {
            if (i + 1 >= v.size())
                return std::cerr << "No input directory given after -i\n", false;
            if (has_i)
                return std::cerr << "2 -i flags\n", false;
            has_i = true;
            a.in = v[++i];
        }
        else if (v[i] == "-o")
        {
            if (i + 1 >= v.size())
                return std::cerr << "No output directory given after -o\n", false;
            if (has_o)
                return std::cerr << "2 -o flags\n", false;
            has_o = true;
            a.out = v[++i];
        }
        else if (v[i][0] != '-')
            break;
        else
            return std::cerr << "Unknown command: " + v[i] + "\n", false;
        ++i;
    }
    for (; i < v.size(); ++i)
        a.cmd.push_back(v[i]);

    if (!has_i)
        return std::cerr << "No input directory(-i)\n", false;
    if (!has_o)
        return std::cerr << "No output directory(-o)\n", false;
    if (a.cmd.empty())
        return std::cerr << "No command to run your program\n", false;
    return true;
}

int main(int argc, char *argv[])
{
    Args a;
    if (!parse(argc, argv, a))
        return 1;

    Stats stats;
    std::vector<std::string> tests;

    if (!load_tests(a.in, a.out, stats, tests))
        return 1;
    std::cout << "Tests:\n";
    for (auto s : tests)
        std::cout << s << "\n";
    if(tests.empty()) std::cout << "None\n";
    std::cout << "Skipped:\n";
    for (auto s : stats.skipped)
        std::cout << s << "\n";
    if(stats.skipped.empty()) std::cout << "None\n";

    runner(a.cmd, tests, stats, 1000, 1024 * 1024, a.in, a.out);

    std::sort(stats.ok.begin(), stats.ok.end());
    std::sort(stats.wa.begin(), stats.wa.end());
    for (auto ok : stats.ok)
    {
        std::cout << ok << " OK\n";
    }
    for (auto wa : stats.wa)
    {
        std::cout << wa << " WA\n";
    }
    return 0;
}
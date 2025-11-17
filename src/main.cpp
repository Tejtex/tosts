#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "load_tests.cpp"
#include "runner.cpp"
#include "utils.cpp"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"




struct Args
{
    std::string in, out;
    float time;
    std::vector<std::string> cmd;
};

void print_help() {
    std::cout <<
R"(tosts — a simple tester for competitive programming

Usage:
  tosts -i <indir> -o <outdir> [-t <timeout>] COMMAND...

Required options:
  -i, --input <indir>       Directory containing input test files
  -o, --output <outdir>     Directory to write output or expected results
  -t, --timeout <timeout>   The timeout for each test

Positional arguments:
  COMMAND...                One or more commands to execute (e.g. test names)

Optional arguments:
  -h, --help                Show this help message and exit

Examples:
  tosts -i tests/in -o tests/out ./a.out
  tosts -i data -o results python3 program.py
)";
}
bool parse(int argc, char *argv[], Args &a)
{
    std::vector<std::string> v(argv + 1, argv + argc);
    bool has_i = false, has_o = false, has_t = false;
    size_t i = 0;
    while (i < v.size())
    {
        if (v[i] == "-h" or v[i] == "--help") {
            print_help();
            exit(0);
        }
        else if (v[i] == "-t" or v[i] == "--timeout") {
            if (i + 1 >= v.size())
                return std::cerr << "No timeout given after -t\n", false;
            if (has_t)
                return std::cerr << "2 -t flags\n", false;
            has_t = true;
            a.time = std::stof(v[++i]);
        }
        else if (v[i] == "-i" or v[i] == "--input")
        {
            if (i + 1 >= v.size())
                return std::cerr << "No input directory given after -i\n", false;
            if (has_i)
                return std::cerr << "2 -i flags\n", false;
            has_i = true;
            a.in = v[++i];
        }
        else if (v[i] == "-o" or v[i] == "--output")
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

    std::cout << "Found " << tests.size() << " tests; skipped " << stats.skipped.size() << " tests\n";
    runner(a.cmd, tests, stats, (a.time * 1000), 1, a.in, a.out);

    std::sort(stats.ok.begin(), stats.ok.end());
    std::sort(stats.wa.begin(), stats.wa.end());
    std::sort(stats.re.begin(), stats.re.end());
    for (auto &wa : stats.wa)
        std::cout << RED << wa << " WA" << RESET << "\n";
    for (auto &re : stats.re)
        std::cout << YELLOW << re << " RE" << RESET << "\n";
    return 0;
}
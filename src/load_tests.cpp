#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include "stats.cpp"

namespace fs = std::filesystem;

bool load_tests(std::string in_dir, std::string out_dir, Stats &stats, std::vector<std::string> &tests)
{
    if (!fs::is_directory(in_dir))
    {
        std::cerr << in_dir << " is not a directory.\n";
        return false;
    }
    if (!fs::is_directory(out_dir))
    {
        std::cerr << out_dir << " is not a directory.\n";
        return false;
    }

    for (auto entry : fs::directory_iterator(in_dir))
    {
        if (entry.is_regular_file())
        {
            std::string name = entry.path().stem().string();
            if (!name.empty() && entry.path().extension() == ".in")
            {
                if (fs::exists(fs::path(out_dir) / (name + ".out")))
                    tests.push_back(std::move(name));
                else
                {
                    stats.skipped.push_back(std::move(name));
                }
            }
        }
    }

    std::sort(tests.begin(), tests.end());
    return true;
}
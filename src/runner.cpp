#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <filesystem>
#include <fstream>
#include "stats.cpp"
#include "process.cpp"

namespace fs = std::filesystem;

bool readTest(const std::string &path, std::string &outContent)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open())
        return false;

    file.seekg(0, std::ios::end);
    outContent.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&outContent[0], outContent.size());
    file.close();

    return true;
}

void runner(const std::vector<std::string> &command, const std::vector<std::string> &tests, Stats &stats, int timeout, int memory_limit, std::string ind, std::string outd)
{
    const int maxCount = 100;

    std::atomic<int> currentCount{0};
    std::mutex cv_mtx;
    std::mutex stats_mtx;
    std::condition_variable cv;
    std::vector<std::thread> threads;

    for (std::string test : tests)
    {

        std::unique_lock<std::mutex>
            lock(cv_mtx);
        cv.wait(lock, [&]()
                { return currentCount < maxCount; });
        currentCount++;
        lock.unlock();

        threads.emplace_back([&, test]()
                             {
            std::string in, out; 
            if (!readTest((fs::path(ind) / (test + ".in")).string(), in) || !readTest((fs::path(outd)/(test + ".out")).string(), out))
            {
                std::lock_guard<std::mutex> lg(stats_mtx);
                stats.skipped.push_back(test);
                currentCount--;
                cv.notify_one();
                return;
            }

            std::pair<std::string, int> res = run(command, in, timeout, memory_limit);

            {
                std::lock_guard<std::mutex> lg(stats_mtx);
                if (res.second == 0)
                {
                    if (res.first == out)
                        stats.ok.push_back(test);
                    else
                        stats.wa.push_back(test);
                }
                else
                {
                    stats.re.push_back(test);
                }
            }

            currentCount--;
            cv.notify_one(); });
    }

    for (auto &t : threads)
    {
        if (t.joinable())
            t.join();
    }
}
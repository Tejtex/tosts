#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <filesystem>
#include <fstream>
#include <queue>
#include <iostream>
#include "stats.cpp"
#include "process.cpp"
#include "utils.cpp"

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
    const size_t NUM_WORKERS = 32;

    std::queue<std::string> taskQueue;
    for (auto& t : tests) taskQueue.push(t);

    std::mutex queueMutex;
    std::condition_variable queueCV;
    bool done = false;

    std::mutex statsMutex;
    auto start = std::chrono::steady_clock::now();

    int numdone = 0;

    auto worker = [&]() {
        while (true) {
            std::string test;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                queueCV.wait(lock, [&] {
                    return !taskQueue.empty() || done;
                });

                if (done && taskQueue.empty())
                    return;

                test = taskQueue.front();
                taskQueue.pop();
queueCV.notify_all();
            }

            std::string in, out;

            if (!readTest((fs::path(ind) / (test + ".in")).string(), in) ||
                !readTest((fs::path(outd) / (test + ".out")).string(), out))
            {
                std::lock_guard<std::mutex> lg(statsMutex);
                stats.skipped.push_back(test);
                continue;
            }

            auto res = run(command, in, timeout, memory_limit);

            {
                std::lock_guard<std::mutex> lg(statsMutex);
                numdone ++;
                auto end = std::chrono::steady_clock::now();
                print_progress(numdone, tests.size(),std::chrono::duration_cast<std::chrono::milliseconds>( end - start ), stats);

                if (res.second == 0) {
                    if (res.first == out) {
                        stats.ok.push_back(test);
                        

                    }
                    else
                        stats.wa.push_back(test);
                } else if(res.second == 1) {
                    stats.tle.push_back(test);
                } else if(res.second == 2) {
                    stats.mle.push_back(test);
                } else {
                    stats.re.push_back(test);
                }
            }
        }
    };

    std::vector<std::thread> workers;
    workers.reserve(NUM_WORKERS);

    for (size_t i = 0; i < NUM_WORKERS; ++i)
        workers.emplace_back(worker);

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        done = false;
    }
    queueCV.notify_all();

    {
        std::unique_lock<std::mutex> lock(queueMutex);
        while (!taskQueue.empty())
            queueCV.wait(lock);
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        done = true;
    }
    queueCV.notify_all();

    for (auto& w : workers)
        w.join();

    std::cout << std::endl;
}
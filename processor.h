#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <future>

struct Task
{
    std::string filename;
    int gain {0};

    std::shared_ptr<std::promise<bool>> promise;
};

class Processor
{
public:
    explicit Processor();
    ~Processor();

public:
    std::future<bool> addTask(const std::string &filename, int gain);

public:
    bool m_exitFlag {false};

    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_signal;

    std::queue<Task> m_queue;

private:
    void run();
    void processTask(const Task &task);
};

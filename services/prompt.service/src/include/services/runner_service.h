#pragma once

#include <memory>

class ExecutorService;

// Runs a thread pool on top of ExecutorService
class RunnerService
{
public:
    RunnerService(ExecutorService &executor, int threads);
    ~RunnerService();

    RunnerService(const RunnerService &) = delete;
    RunnerService &operator=(const RunnerService &) = delete;

    void start();
    void stop();
    void join();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

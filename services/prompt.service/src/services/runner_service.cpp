#include "services/runner_service.h"
#include "services/executor_service.h"

#include <vector>
#include <thread>
#include <spdlog/spdlog.h>

class RunnerService::Impl
{
public:
    Impl(ExecutorService &executor, int threads)
        : _executor(executor), _threads(threads) {}

    void start()
    {
        spdlog::info("RunnerService: starting {} worker threads", _threads);
        _workers.reserve(_threads);
        for (int i = 0; i < _threads; ++i)
        {
            _workers.emplace_back(
                [this, i]()
                {
                    spdlog::debug("RunnerService: worker {} started", i);
                    _executor.run();
                    spdlog::debug("RunnerService: worker {} stopped", i);
                });
        }
    }

    void stop()
    {
        spdlog::info("RunnerService: stop requested");
        _executor.stop();
    }

    void join()
    {
        for (auto &t : _workers)
        {
            if (t.joinable())
            {
                t.join();
            }
        }
        _workers.clear();
    }

private:
    ExecutorService &_executor;
    int _threads{};
    std::vector<std::thread> _workers{};
};

RunnerService::RunnerService(ExecutorService &executor, int threads) : _impl(std::make_unique<Impl>(executor, threads)) {}
RunnerService::~RunnerService() = default;
void RunnerService::start() { _impl->start(); }
void RunnerService::stop() { _impl->stop(); }
void RunnerService::join() { _impl->join(); }

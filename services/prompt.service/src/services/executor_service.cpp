#include "services/executor_service.h"

#include <boost/asio/executor_work_guard.hpp>
#include <spdlog/spdlog.h>

class ExecutorService::Impl
{
public:
    Impl()
        : _context(),
          _work_guard(boost::asio::make_work_guard(_context)) {}

    boost::asio::io_context &context() { return _context; }

    void run()
    {
        spdlog::debug("ExecutorService: io_context::run() enter");
        try
        {
            _context.run();
        }
        catch (const std::exception &ex)
        {
            spdlog::error("ExecutorService: exception in io_context run: {}", ex.what());
        }
        spdlog::debug("ExecutorService: io_context::run() exit");
    }

    void stop()
    {
        spdlog::info("ExecutorService: stop requested");
        _work_guard.reset();
        _context.stop();
    }

private:
    boost::asio::io_context _context;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> _work_guard;
};

ExecutorService::ExecutorService() : _impl(std::make_unique<Impl>()) {}
ExecutorService::~ExecutorService() = default;
boost::asio::io_context &ExecutorService::context() { return _impl->context(); }
void ExecutorService::run() { _impl->run(); }
void ExecutorService::stop() { _impl->stop(); }

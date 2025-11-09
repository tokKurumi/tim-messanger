#pragma once

#include <memory>
#include <boost/asio/io_context.hpp>

// Io multiplexing executor based on Boost.Asio io_context
class ExecutorService
{
public:
    ExecutorService();
    ~ExecutorService();

    ExecutorService(const ExecutorService &) = delete;
    ExecutorService &operator=(const ExecutorService &) = delete;

    boost::asio::io_context &context();

    void run();
    void stop();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

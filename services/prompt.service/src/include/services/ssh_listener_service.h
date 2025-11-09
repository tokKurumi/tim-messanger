#pragma once

#include <memory>
#include <boost/asio/ip/tcp.hpp>

struct SshConfig;
class ExecutorService;

// Accepts TCP connections and upgrades them to SSH sessions
class SshListenerService
{
public:
    SshListenerService(ExecutorService &executor, const SshConfig &cfg);
    ~SshListenerService();

    SshListenerService(const SshListenerService &) = delete;
    SshListenerService &operator=(const SshListenerService &) = delete;

    void start();
    void stop();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

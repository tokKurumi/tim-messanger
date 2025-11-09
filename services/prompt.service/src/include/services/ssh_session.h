#pragma once

#include <memory>
#include <boost/asio/ip/tcp.hpp>

struct SshConfig;

// Represents a single SSH session lifecycle
class SshSession
{
public:
    SshSession(boost::asio::ip::tcp::socket socket, const SshConfig &cfg);
    ~SshSession();

    SshSession(const SshSession &) = delete;
    SshSession &operator=(const SshSession &) = delete;

    void start();
    void stop();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

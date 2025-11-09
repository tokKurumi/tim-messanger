#include "services/ssh_listener_service.h"
#include "services/executor_service.h"
#include "services/ssh_session.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/dispatch.hpp>
#include <spdlog/spdlog.h>

#include "configuration/config.h"

class SshListenerService::Impl
{
public:
    Impl(ExecutorService &executor, const SshConfig &cfg)
        : _executor(executor), _cfg(cfg), _acceptor(executor.context()) {}

    void start()
    {
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::make_address(_cfg.bind_addr), _cfg.bind_port);
        boost::system::error_code ec;

        _acceptor.open(ep.protocol(), ec);
        if (ec)
        {
            spdlog::error("SshListener: open failed: {}", ec.message());
            return;
        }

        _acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
        if (ec)
        {
            spdlog::error("SshListener: reuse_address failed: {}", ec.message());
        }

        _acceptor.bind(ep, ec);
        if (ec)
        {
            spdlog::error("SshListener: bind failed: {}", ec.message());
            return;
        }

        _acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec)
        {
            spdlog::error("SshListener: listen failed: {}", ec.message());
            return;
        }

        spdlog::info("SshListener: listening on {}:{}", _cfg.bind_addr, _cfg.bind_port);
        do_accept();
    }

    void stop()
    {
        boost::system::error_code ec;
        _acceptor.close(ec);
    }

private:
    void do_accept()
    {
        _acceptor.async_accept(
            [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
            {
                if (!ec)
                {
                    auto session = std::make_shared<SshSession>(std::move(socket), _cfg);
                    session->start(); // immediate start; currently will close
                }
                else
                {
                    if (ec != boost::asio::error::operation_aborted)
                    {
                        spdlog::warn("SshListener: accept error: {}", ec.message());
                    }
                }
                if (_acceptor.is_open())
                {
                    do_accept();
                }
            });
    }

private:
    ExecutorService &_executor;
    SshConfig _cfg;
    boost::asio::ip::tcp::acceptor _acceptor;
};

SshListenerService::SshListenerService(ExecutorService &executor, const SshConfig &cfg) : _impl(std::make_unique<Impl>(executor, cfg)) {}
SshListenerService::~SshListenerService() = default;
void SshListenerService::start() { _impl->start(); }
void SshListenerService::stop() { _impl->stop(); }

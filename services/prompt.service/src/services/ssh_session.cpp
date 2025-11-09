#include "services/ssh_session.h"

#include <libssh/libssh.h>
#include <libssh/server.h>
#include <spdlog/spdlog.h>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <unistd.h>

#include "configuration/config.h"

class SshSession::Impl
{
public:
    Impl(boost::asio::ip::tcp::socket socket, const SshConfig &cfg)
        : _socket(std::move(socket)), _cfg(cfg) {}

    ~Impl() { stop(); }

    void start()
    {
        spdlog::info("SshSession: new connection from {}", _socket.remote_endpoint().address().to_string());
        if (_cfg.rsa_host_private_key_path.empty())
        {
            spdlog::error("SshSession: RSA private host key path not configured (PROMPT_SSH_RSA_HOST_PRIVATE_KEY_PATH)");
            stop();
            return;
        }

        // Duplicate socket file descriptor to transfer ownership from Asio to libssh
        int orig_fd = _socket.native_handle();
        int fd = ::dup(orig_fd);
        if (fd < 0)
        {
            spdlog::error("SshSession: dup(fd) failed");
            stop();
            return;
        }

        // Close Asio TCP socket; fd is now managed by libssh
        {
            boost::system::error_code ec;
            _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            _socket.close(ec);
        }

        // Create ssh_bind and ssh_session
        _bind = ssh_bind_new();
        if (!_bind)
        {
            spdlog::error("SshSession: ssh_bind_new() returned nullptr");
            stop();
            return;
        }

        // Set RSA private key for SSH server
        if (ssh_bind_options_set(_bind, SSH_BIND_OPTIONS_RSAKEY, _cfg.rsa_host_private_key_path.c_str()) != SSH_OK)
        {
            spdlog::error("SshSession: failed to set RSA host key: {}", _cfg.rsa_host_private_key_path);
            stop();
            return;
        }

        _session = ssh_new();
        if (!_session)
        {
            spdlog::error("SshSession: ssh_new() returned nullptr");
            stop();
            return;
        }
        ssh_set_blocking(_session, 0);

        // Bind accepted TCP connection to libssh session
        int rc = ssh_bind_accept_fd(_bind, _session, fd);
        if (rc != SSH_OK)
        {
            spdlog::error("SshSession: ssh_bind_accept_fd() failed with rc={}", rc);
            stop();
            return;
        }

        // Wrap fd in Asio descriptor for async IO readiness notifications
        _descriptor = std::make_unique<boost::asio::posix::stream_descriptor>(_socket.get_executor());
        _descriptor->assign(fd);

        drive_handshake();
    }

    void stop()
    {
        if (_descriptor)
        {
            // Release ownership; libssh will close the fd
            (void)_descriptor->release();
            _descriptor.reset();
        }
        if (_session)
        {
            ssh_disconnect(_session);
            ssh_free(_session);
            _session = nullptr;
        }
        if (_bind)
        {
            ssh_bind_free(_bind);
            _bind = nullptr;
        }
        if (_socket.is_open())
        {
            boost::system::error_code ec;
            _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            _socket.close(ec);
        }
    }

private:
    void wait_io()
    {
        if (!_descriptor)
            return;
        // Wait for socket readiness
        _descriptor->async_wait(
            boost::asio::posix::stream_descriptor::wait_read,
            [this](const boost::system::error_code &ec)
            {
                if (ec)
                {
                    if (ec != boost::asio::error::operation_aborted)
                    {
                        spdlog::warn("SshSession: IO wait error: {}", ec.message());
                    }

                    stop();
                    return;
                }

                drive_handshake();
            });
    }

    void drive_handshake()
    {
        if (!_session)
        {
            stop();
            return;
        }

        int rc = ssh_handle_key_exchange(_session);
        if (rc == SSH_OK)
        {
            spdlog::info("SshSession: key exchange completed");

            // Send a simple greeting to the client over a session channel.
            // This is a minimal friendly banner before implementing full
            // authentication and TUI. If the channel cannot be opened we
            // continue to shutdown the session.
            ssh_channel channel = ssh_channel_new(_session);
            if (channel)
            {
                int rc = ssh_channel_open_session(channel);
                if (rc == SSH_OK)
                {
                    const char *msg = "hello\n";
                    int written = ssh_channel_write(channel, msg, (uint32_t)strlen(msg));
                    if (written < 0)
                    {
                        spdlog::warn("SshSession: failed to write greeting to channel");
                    }
                    else
                    {
                        spdlog::info("SshSession: sent greeting ({} bytes)", written);
                    }
                    // politely close channel
                    ssh_channel_send_eof(channel);
                    ssh_channel_close(channel);
                }
                else
                {
                    spdlog::warn("SshSession: failed to open session channel: rc={}", rc);
                }
                ssh_channel_free(channel);
            }
            else
            {
                spdlog::warn("SshSession: ssh_channel_new() returned nullptr");
            }

            // TODO: authentication (password/token) and full TUI
            stop();
            return;
        }

        if (rc == SSH_AGAIN)
        {
            // Wait for socket readiness and retry
            wait_io();
            return;
        }

        spdlog::warn("SshSession: key exchange failed with rc={}", rc);
        stop();
    }

    boost::asio::ip::tcp::socket _socket;
    SshConfig _cfg;
    ssh_bind _bind{nullptr};
    ssh_session _session{nullptr};
    std::unique_ptr<boost::asio::posix::stream_descriptor> _descriptor{};
};

SshSession::SshSession(boost::asio::ip::tcp::socket socket, const SshConfig &cfg) : _impl(std::make_unique<Impl>(std::move(socket), cfg)) {}
SshSession::~SshSession() = default;
void SshSession::start() { _impl->start(); }
void SshSession::stop() { _impl->stop(); }

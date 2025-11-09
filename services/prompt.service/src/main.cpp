#include <iostream>
#include <string>

#include <libssh/libssh.h>
#include <libssh/server.h>
#include <boost/asio/signal_set.hpp>
#include <spdlog/spdlog.h>

#include "configuration/config.h"
#include "services/executor_service.h"
#include "services/runner_service.h"
#include "services/ssh_listener_service.h"

int main()
{
    ConfigLoader cfg_loader;
    AppConfig config = cfg_loader.load();

    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] %v");
    spdlog::info("Starting prompt.service...");
    spdlog::info("SSH Bind Address: {}", config.ssh.bind_addr);
    spdlog::info("SSH Bind Port: {}", config.ssh.bind_port);
    spdlog::info("SSH Host Public Key: {}", config.ssh.host_public_key);
    spdlog::info("SSH Host Key Path: {}", config.ssh.rsa_host_private_key_path);
    spdlog::info("Thread Pool Size: {}", config.pool.threads);

    ExecutorService executor;
    RunnerService runner(executor, config.pool.threads);
    SshListenerService listener(executor, config.ssh);

    // Start IO threads
    runner.start();

    // Start accepting connections
    listener.start();

    // Handle SIGINT/SIGTERM for graceful shutdown
    boost::asio::signal_set signals(executor.context(), SIGINT, SIGTERM);
    signals.async_wait(
        [&](const boost::system::error_code &, int signo)
        {
            spdlog::info("Signal {} received: shutting down listener and executor", signo);
            listener.stop();
            runner.stop();
        });

    // Block main thread until io_context stops
    executor.run();

    // Join worker threads
    runner.join();

    spdlog::info("prompt.service stopped");
    return 0;
}

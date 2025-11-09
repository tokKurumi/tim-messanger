#pragma once

#include <cstdint>
#include <string>
#include <memory>

struct SshConfig
{
    // PROMPT_BIND_ADDR
    // address to bind SSH server
    std::string bind_addr;

    // PROMPT_BIND_PORT
    // port to bind SSH server
    std::uint16_t bind_port;

    // PROMPT_SSH_RSA_HOST_PUBLIC_KEY_PATH
    // path to host public key
    std::string rsa_host_public_key_path;

    // PROMPT_SSH_RSA_HOST_PRIVATE_KEY_PATH
    // path to RSA private host key file for SSH server
    std::string rsa_host_private_key_path;
};

struct ThreadPoolConfig
{
    // PROMPT_THREADS
    // number of threads in the pool
    int threads;
};

struct AppConfig
{
    SshConfig ssh;
    ThreadPoolConfig pool;
};

// Lightweight env config loader class
class ConfigLoader
{
public:
    ConfigLoader();
    ~ConfigLoader();

    ConfigLoader(const ConfigLoader &) = delete;
    ConfigLoader &operator=(const ConfigLoader &) = delete;

    AppConfig load();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

#pragma once

#include <cstdint>
#include <string>
#include <memory>

struct SshConfig
{
    // PROMPT_BIND_ADDR
    // address to bind SSH server
    std::string bind_addr{"0.0.0.0"};

    // PROMPT_BIND_PORT
    // port to bind SSH server
    std::uint16_t bind_port{2222};

    // PROMPT_SSH_RSA_HOST_PUBLIC_KEY_PATH (optional)
    // path to host public key (optional; used for logging / distribution to clients)
    std::string rsa_host_public_key_path;

    // PROMPT_SSH_RSA_HOST_PRIVATE_KEY_PATH
    // path to RSA private host key file for SSH server
    // can be set via env var or as docker secret path (e.g., /run/secrets/prompt_ssh_rsa_key)
    std::string rsa_host_private_key_path;
};

struct ThreadPoolConfig
{
    // PROMPT_THREADS
    // number of threads in the pool
    int threads{4};
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

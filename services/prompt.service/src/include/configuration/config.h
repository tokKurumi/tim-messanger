#pragma once

#include <cstdint>
#include <string>

struct SshConfig
{
    // PROMPT_BIND_ADDR
    // address to bind SSH server
    std::string bind_addr{"0.0.0.0"};

    // PROMPT_BIND_PORT
    // port to bind SSH server
    std::uint16_t bind_port{2222};

    // PROMPT_SSH_HOST_PUBLIC_KEY
    // persistent host public key
    std::string host_public_key;
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

AppConfig load_config_from_env();

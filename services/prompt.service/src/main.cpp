#include <iostream>
#include <string>

#include <libssh/libssh.h>
#include <libssh/server.h>

#include "configuration/config.h"

int main()
{
    AppConfig config = load_config_from_env();

    std::cout << "Starting prompt.service..." << std::endl;

    std::cout << "SSH Bind Address: " << config.ssh.bind_addr << std::endl;
    std::cout << "SSH Bind Port: " << config.ssh.bind_port << std::endl;
    std::cout << "SSH Host Public Key: " << config.ssh.host_public_key << std::endl;
    std::cout << "Thread Pool Size: " << config.pool.threads << std::endl;

    return 0;
}

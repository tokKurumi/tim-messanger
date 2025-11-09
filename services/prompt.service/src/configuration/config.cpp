#include <cstdlib>
#include <stdexcept>

#include <boost/lexical_cast.hpp>

#include "configuration/config.h"

class ConfigLoader::Impl
{
public:
    Impl() = default;

    AppConfig load()
    {
        AppConfig cfg{};

        if (const char *address = std::getenv("PROMPT_BIND_ADDR"))
        {
            if (*address)
            {
                cfg.ssh.bind_addr = address;
            }
        }

        if (const char *port_str = std::getenv("PROMPT_BIND_PORT"))
        {
            try
            {
                auto p = boost::lexical_cast<long>(port_str);
                if (p >= 0 && p <= 65535)
                {
                    cfg.ssh.bind_port = static_cast<std::uint16_t>(p);
                }
            }
            catch (const boost::bad_lexical_cast &)
            {
            }
        }

        if (const char *pub_path = std::getenv("PROMPT_SSH_RSA_HOST_PUBLIC_KEY_PATH"))
        {
            if (*pub_path)
            {
                cfg.ssh.rsa_host_public_key_path = pub_path;
            }
        }

        if (const char *priv_path = std::getenv("PROMPT_SSH_RSA_HOST_PRIVATE_KEY_PATH"))
        {
            if (*priv_path)
            {
                cfg.ssh.rsa_host_private_key_path = priv_path;
            }
        }

        if (const char *threads_str = std::getenv("PROMPT_THREADS"))
        {
            try
            {
                auto n = boost::lexical_cast<int>(threads_str);
                if (n > 0)
                {
                    cfg.pool.threads = n;
                }
            }
            catch (const boost::bad_lexical_cast &)
            {
            }
        }

        return cfg;
    }
};

ConfigLoader::ConfigLoader() : _impl(std::make_unique<Impl>()) {}
ConfigLoader::~ConfigLoader() = default;
AppConfig ConfigLoader::load() { return _impl->load(); }

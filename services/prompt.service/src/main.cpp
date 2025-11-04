#include <iostream>
#include <string>

#include <libssh/libssh.h>
#include <libssh/server.h>

int main() {
    const std::string bind_port = "2222";

    ssh_bind sshbind = ssh_bind_new();
    if (!sshbind) {
        std::cerr << "[ssh] Failed to create ssh_bind" << std::endl;
        return 1;
    }

    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDADDR, "0.0.0.0");
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT_STR, bind_port.c_str());

    // Generate an ephemeral ED25519 host key in memory so we don't rely on system key files.
    // This is NOT suitable for production use since clients will get a different host key on each restart or replication.
    ssh_key hostkey = nullptr;
    if (ssh_pki_generate(SSH_KEYTYPE_ED25519, 0, &hostkey) != SSH_OK || hostkey == nullptr) {
        std::cerr << "[ssh] Failed to generate ephemeral host key: " << ssh_get_error(sshbind) << std::endl;
        ssh_bind_free(sshbind);
        return 1;
    }
    if (ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_IMPORT_KEY, hostkey) != SSH_OK) {
        std::cerr << "[ssh] Failed to import host key: " << ssh_get_error(sshbind) << std::endl;
        ssh_key_free(hostkey);
        ssh_bind_free(sshbind);
        return 1;
    }

    if (ssh_bind_listen(sshbind) != SSH_OK) {
        std::cerr << "[ssh] Listen error: " << ssh_get_error(sshbind) << std::endl;
        ssh_key_free(hostkey);
        ssh_bind_free(sshbind);
        return 1;
    }

    std::cout << "SSH server listening on 0.0.0.0:" << bind_port << " (ephemeral host key)" << std::endl;

    while (true) {
        ssh_session session = ssh_new();
        if (!session) {
            std::cerr << "[ssh] Failed to allocate session" << std::endl;
            continue;
        }

        if (ssh_bind_accept(sshbind, session) == SSH_ERROR) {
            std::cerr << "[ssh] Accept error: " << ssh_get_error(sshbind) << std::endl;
            ssh_free(session);
            continue;
        }

        if (ssh_handle_key_exchange(session) != SSH_OK) {
            std::cerr << "[ssh] KEX failed: " << ssh_get_error(session) << std::endl;
            ssh_disconnect(session);
            ssh_free(session);
            continue;
        }

        // Minimal auth handling: accept any password or public key; otherwise advertise methods.
        bool authed = false;
        while (!authed) {
            ssh_message msg = ssh_message_get(session);
            if (!msg) break; // client closed

            if (ssh_message_type(msg) == SSH_REQUEST_AUTH) {
                int subtype = ssh_message_subtype(msg);
                if (subtype == SSH_AUTH_METHOD_PASSWORD || subtype == SSH_AUTH_METHOD_PUBLICKEY) {
                    // Accept everything for MVP
                    ssh_message_auth_reply_success(msg, 0);
                    authed = true;
                } else {
                    ssh_message_auth_set_methods(msg, SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY);
                    ssh_message_reply_default(msg);
                }
            } else {
                ssh_message_reply_default(msg);
            }
            ssh_message_free(msg);
        }

        std::cout << "[ssh] Connection handled (authed=" << (authed ? "yes" : "no") << "). Closing session." << std::endl;
        ssh_disconnect(session);
        ssh_free(session);
    }

    ssh_key_free(hostkey);
    ssh_bind_free(sshbind);
    return 0;
}
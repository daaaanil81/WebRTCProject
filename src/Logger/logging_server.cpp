#include <ace/FILE_Addr.h>
#include <ace/FILE_Connector.h>
#include <ace/FILE_IO.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Stream.h>

#include <string>

#include "Logger/logging_server.h"

int Logging_Server::run(int argc, char *argv[]) {

    if (open(argc > 1 ? std::atoi(argv[1]) : 0) == -1)
        return -1;

    for (;;) {
        if (wait_for_multiple_events() == -1) {
            return -1;
        }

        if (handle_connections() == -1) {
            return -1;
        }

        if (handle_data() == -1) {
            return -1;
        }
    }
    return 0;
}

int Logging_Server::open(u_short logger_port) {

    ACE::set_handle_limit();

    ACE_INET_Addr server_addr;
    int result = 0;

    if (logger_port != 0) {
        result = server_addr.set(logger_port, INADDR_ANY);
    } else {
        result = server_addr.set("ace_logger", INADDR_ANY);
    }

    if (result == -1) {
        return -1;
    }

    return acceptor_.open(server_addr, 1);
}

int Logging_Server::make_log_file(ACE_FILE_IO &logging_file,
                                  ACE_SOCK_Stream *logging_peer) {
    std::string filename;

    filename.reserve(MAXHOSTNAMELEN + sizeof(".log"));

    if (logging_peer != nullptr) {
        ACE_INET_Addr logging_peer_addr;
        logging_peer->get_remote_addr(logging_peer_addr);
        logging_peer_addr.get_host_addr(&filename[0], MAXHOSTNAMELEN);
        filename += ".log";
    } else {
        filename = "logging_server.log";
    }

    ACE_FILE_Connector connector;

    return connector.connect(logging_file, ACE_FILE_Addr(filename.c_str()), 0,
                             ACE_Addr::sap_any, 0, O_RDWR | O_CREAT | O_APPEND,
                             ACE_DEFAULT_FILE_PERMS);
}

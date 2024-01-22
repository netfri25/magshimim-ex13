#pragma once

#include <string>

// TODO: proper exceptions

class Connection {
public:
    // TODO: how do I disallow copy/move constructors?
    Connection() = delete;

    ~Connection();

    // send the message
    void send_raw(std::string const& data, int const flags = 0);

    // recv the amount of bytes passed as the argument
    std::string read_raw(unsigned const amount, int const flags = 0);

protected:
    Connection(int const domain, int const type, int const protocol);

    // connect to a given server address
    void connect(std::string const& server_ip, unsigned short const port);

    // bind the socket to a port
    void bind(unsigned short const port, int in_family, int in_addr);

    // listen to a given number of connections
    void listen(unsigned short const connections);

    // accept a connection
    Connection accept();

private:
    Connection(int socket_fd);

protected:
    int _socket_fd;
};

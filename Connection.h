#pragma once

#include <string>

// NOTE: not using const for methods ON PURPOSE
//       all of the methods are doing some sort of a side effect, so marking them as const doesn't make any sense

class Connection {
public:
    // disallow copy/move/default constructors
    Connection() = delete;
    Connection(Connection&&) = default;
    Connection(Connection const&) = delete;

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

    // close the connection
    void close();

private:
    Connection(int socket_fd);

protected:
    int _socket_fd;
};

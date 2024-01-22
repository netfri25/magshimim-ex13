#pragma once

#include <memory>
#include <string>

#define _DEBUG

#ifdef _DEBUG
#define TRACE(arg) ::std::cerr << "[TRACE]: " arg << std::endl
#else
#define TRACE(arg)
#endif

// NOTE: not using const for methods ON PURPOSE
//       all of the methods are doing some sort of a side effect, so marking them as const doesn't make any sense

class Message;

class Connection {
public:
    // disallow copy/move/default constructors
    Connection() = delete;
    Connection(Connection&&) = default;
    Connection(Connection const&) = default;

    // send a raw message
    void send_raw(std::string const& data, int const flags = 0);

    // send message
    void send(Message const& msg);

    // recv the amount of bytes passed as the argument
    std::string read_raw(unsigned const amount, int const flags = 0);

    // read message
    std::unique_ptr<Message> read();

    // close the connection
    void close() const;

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

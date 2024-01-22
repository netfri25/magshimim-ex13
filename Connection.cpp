#include "Connection.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

Connection::Connection(int const domain, int const type, int const protocol) {
    this->_socket_fd = ::socket(domain, type, protocol);

    // TODO: exceptions
    if (this->_socket_fd < 0)
        throw std::runtime_error(std::string("Connection(): ") + strerror(errno));
}

Connection::Connection(int fd) {
    this->_socket_fd = fd;
}

void Connection::connect(std::string const& server_ip, unsigned short const port) {
	struct sockaddr_in sa = {0};

	sa.sin_port = htons(port); // port that server will listen to
	sa.sin_family = AF_INET;   // must be AF_INET
	sa.sin_addr.s_addr = inet_addr(server_ip.c_str());    // the IP of the server

	// the process will not continue until the server accepts the client
	int const status = ::connect(this->_socket_fd, (struct sockaddr*)&sa, sizeof(sa));

    // TODO: exceptions
	if (status < 0)
		throw std::runtime_error(std::string("connect error: ") + strerror(errno));
}


void Connection::bind(unsigned short const port, int in_family, int in_addr) {
    struct sockaddr_in sa = {0};

    sa.sin_port = htons(port);
    sa.sin_family = in_family;
    sa.sin_addr.s_addr = in_addr;
    // sa.sin_family = AF_INET;
    // sa.sin_addr.s_addr = INADDR_ANY;

    // TODO: exceptions
    if (::bind(this->_socket_fd, (struct sockaddr*) &sa, sizeof(sa)) < 0)
        throw std::runtime_error(std::string("bind error: ") + strerror(errno));
}

void Connection::listen(unsigned short const connections) {
    // TODO: exceptions
    if (::listen(this->_socket_fd, connections) < 0)
        throw std::runtime_error(std::string("listen error: ") + strerror(errno));
}

Connection Connection::accept() {
    int fd = ::accept(this->_socket_fd, NULL, NULL);
    // TODO: exceptions
    if (fd < 0)
        throw std::runtime_error(std::string("accept error: ") + strerror(errno));
    std::cerr << "accepted: " << fd << std::endl;
    return Connection(fd);
}

void Connection::send_raw(std::string const& data, int const flags) {
    int const sent = send(this->_socket_fd, data.c_str(), data.size(), flags);
    // TODO: exceptions
    if (sent < 0)
        throw std::runtime_error(std::string("send error: ") + strerror(errno));
}

std::string Connection::read_raw(unsigned const amount, int const flags) {
    char* buf = new char[amount + 1];
    int const read_count = recv(this->_socket_fd, buf, amount, flags);
    // TODO: exceptions
    if (read_count < 0) {
        delete[] buf;
        throw std::runtime_error(std::string("read error: ") + strerror(errno));
    }

    std::string const output(buf, read_count);
    delete[] buf;
    return output;
}

void Connection::close() const {
    std::cerr << "[WARN]: closing fd " << this->_socket_fd << std::endl;
    ::close(this->_socket_fd);
}

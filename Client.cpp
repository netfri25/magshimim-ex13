#include "Client.h"

#include <exception>
#include <memory>
#include <string>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <iomanip>

Client::Client(std::string const& username)
    : Connection(AF_INET, SOCK_STREAM, IPPROTO_TCP)
    , _username(username)
{ }

void Client::connect(std::string const& server_ip, int const port) {
    this->Connection::connect(server_ip, port);
    this->login_notify();
}

void Client::login_notify() {
    this->send_message(MClientLogIn(this->_username));
    // auto const response = MServerUpdate(*this);
}

void Client::send_message(Message const& message) {
    this->send_raw(message.encode());
}

void Client::interact() {
    // TODO: interaction
    for (;;);
}

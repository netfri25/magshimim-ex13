#pragma once

#include "Connection.h"
#include "Message.h"

#include <string>
#include <netinet/in.h>
#include <sys/socket.h>

class Client : protected Connection {
public:
	Client(std::string const& username);

	void connect(std::string const& server_ip, int const port);
    void login_notify();
    void send_message(Message const& message);
    void interact();

private:
    std::string _username;
};

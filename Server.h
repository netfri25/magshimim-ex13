#pragma once

#include "Connection.h"
#include "Message.h"

#include <mutex>
#include <queue>
#include <condition_variable>
#include <set>

class Server : protected Connection {
public:
	Server();
	void serve(unsigned short const port);

private:
	void accept_client();
	void handle_client(Connection client_connection);
    MServerUpdate current_status(std::string const& username);

    // std::condition_variable _new_msg;
    std::mutex _connected_users_mtx;
    std::set<std::string> _connected_users;
};

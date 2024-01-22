#include "Server.h"
#include "Message.h"

#include <arpa/inet.h>
#include <iostream>
#include <mutex>
#include <sys/socket.h>
#include <thread>

Server::Server()
    : Connection(AF_INET, SOCK_STREAM, IPPROTO_TCP)
    , _connected_users()
{ }

void Server::serve(unsigned short const port) {
    this->bind(port, AF_INET, INADDR_ANY);
    this->listen(SOMAXCONN);

    std::thread([this] {
        try {
            // ;; == ever
            // for (ever)
            for (;;) this->accept_client();
        } catch (std::exception const& e) {
            std::cerr << "[ERROR]: unable to accept client: " << e.what() << std::endl;
        }
    }).detach();

    // handle messages queue
    for (;;) {
    }

    this->close();
}

void Server::accept_client() {
    Connection connection = this->accept();
    std::thread([this, connection] {
        this->handle_client(connection);
        connection.close();
    }).detach();
}

void Server::handle_client(Connection client_connection) {
    // login
    TRACE("Client has connected! ");

    std::unique_ptr<Message> first_msg = client_connection.read();
    MClientLogIn const& login_msg = dynamic_cast<MClientLogIn const&>(*first_msg);
    TRACE("Client name: " << login_msg.username);

    std::string const client_username = std::move(login_msg.username);

    this->_connected_users_mtx.lock();
    this->_connected_users.insert(client_username);
    this->_connected_users_mtx.unlock();

    {
        this->_connected_users_mtx.lock();
        MServerUpdate const status("", client_username, this->_connected_users);
        this->_connected_users_mtx.unlock();
        client_connection.send(status);
    }

    // interaction
    for (;;) {
        std::unique_ptr<Message> msg = client_connection.read();
        switch (msg->get_message_type()) {
        case MT_CLIENT_UPDATE: {
            MClientUpdate const& client_update = dynamic_cast<MClientUpdate const&>(*first_msg);
            // TODO: client update
            throw std::runtime_error("TODO: client update");
        } break;

        case MT_CLIENT_FINISH:
        case MT_CLIENT_EXIT:
        case MT_CLIENT_LOG_IN:
        case MT_SERVER_UPDATE:
        // TODO: exceptions
        default:
            throw std::runtime_error("interaction error: invalid message from client");
        }
    }
}

MServerUpdate Server::current_status(std::string const& username) {
    std::lock_guard<std::mutex> guard(this->_connected_users_mtx);
    return MServerUpdate("", username, this->_connected_users);
}

#include "Server.h"
#include "Message.h"

#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <mutex>
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
            this->accept_client();
        } catch (std::exception const& e) {
            std::cerr << "[ERROR]: unable to accept client: " << e.what() << std::endl;
        }
    }).detach();

    // handle messages queue
    for (;;) {
        std::unique_lock<std::mutex> msgs_locker(this->_msgs_mtx);
        this->_new_msg.wait(msgs_locker);
        msgs_locker.unlock();
        TRACE("got a new message");

        msgs_locker.lock();
        while (!this->_msgs_queue.empty()) {
            ChatMessage const msg = this->_msgs_queue.front();
            this->_msgs_queue.pop();
            msgs_locker.unlock();
            TRACE("handling message " << std::to_string(msg));
            std::string const filename = msg.author > msg.recipient ? msg.recipient + '&' + msg.author : msg.author + '&' + msg.recipient;
            std::ofstream file(filename + ".txt", std::ios::app);
            file << std::to_string(msg) << std::flush;
            file.close();
            msgs_locker.lock();
        }
        msgs_locker.unlock();

        TRACE("handled all the messages");
    }

    this->close();
}

void Server::accept_client() {
    for (;;) {
        Connection connection = this->accept();
        std::thread([this, connection] {
            this->handle_client(connection);
            connection.close();
        }).detach();
    }
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
    std::set<std::string> connected = this->_connected_users;
    this->_connected_users_mtx.unlock();

    client_connection.send(MServerUpdate("", client_username, connected));

    // interaction
    bool keep_running = true;
    while (keep_running) {
        std::unique_ptr<Message> msg = client_connection.read();
        if (msg == NULL) {
            TRACE("client \"" << client_username << "\" disconnected");
            break;
        }

        switch (msg->get_message_type()) {
        case MT_CLIENT_UPDATE: {
            MClientUpdate const& client_update = dynamic_cast<MClientUpdate&>(*msg);

            ChatMessage const msg(client_username, client_update.recipient, client_update.message);

            if (client_update.recipient.empty() || client_update.message.empty()) {
                std::string const filename = msg.author > msg.recipient ? msg.recipient + '&' + msg.author : msg.author + '&' + msg.recipient;
                std::ofstream file(filename + ".txt", std::ios::in);
                std::stringstream s;
                s << file.rdbuf();
                std::string const chat = s.str();
                this->_connected_users_mtx.lock();
                std::set<std::string> connected = this->_connected_users;
                this->_connected_users_mtx.unlock();
                TRACE("chat: " << chat);

                client_connection.send(MServerUpdate(chat, msg.recipient, connected));
                continue;
            }

            {
                this->_msgs_mtx.lock();
                this->_msgs_queue.push(msg);
                this->_msgs_mtx.unlock();
                TRACE("added" << std::to_string(msg) << " to the queue");
            }

            this->_new_msg.notify_one();
            TRACE("notified about the latest message");

            client_connection.send(MServerUpdate(std::to_string(msg), msg.recipient, connected));
        } break;

        case MT_CLIENT_FINISH:
        case MT_CLIENT_EXIT:
            keep_running = false;
            break;

        case MT_CLIENT_LOG_IN:
        case MT_SERVER_UPDATE:
        // TODO: exceptions
        default:
            throw std::runtime_error("interaction error: invalid message from client");
        }
    }
}

MServerUpdate Server::current_status(std::string const& username) {
    return MServerUpdate("", username, this->_connected_users);
}

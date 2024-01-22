#pragma once

#include <memory>
#include <string>
#include <set>

class Connection;

enum MessageType : uint8_t {
    MT_CLIENT_LOG_IN = 200,
    MT_CLIENT_UPDATE = 204,
    MT_CLIENT_FINISH = 207,
    MT_CLIENT_EXIT = 208,
    MT_SERVER_UPDATE = 101,
};

class Message {
public:
    virtual ~Message() = 0;
    virtual std::string encode() const = 0;
    static std::unique_ptr<Message> decode(Connection& conn);
};

class MClientLogIn : public Message {
public:
    MClientLogIn(Connection& conn);
    MClientLogIn(std::string const& username);

    virtual std::string encode() const override;

private:
    std::string _username;
};

class MClientUpdate : public Message {
public:
    MClientUpdate(Connection& conn);
    MClientUpdate(std::string const& recipient, std::string const& message);

    virtual std::string encode() const override;

private:
    std::string _recipient;
    std::string _message;
};

class MClientFinish {
};

class MClientExit {
};

class MServerUpdate : public Message {
public:
    MServerUpdate(Connection& conn);
    MServerUpdate(
        std::string const& chat_content,
        std::string const& recipient,
        std::set<std::string> const& _connected_users
    );

    virtual std::string encode() const override;

private:
    std::string _chat_content;
    std::string _recipient;
    std::set<std::string> _connected_users;
};

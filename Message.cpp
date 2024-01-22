#include "Message.h"
#include "Connection.h"

#include <iomanip>
#include <memory>

std::unique_ptr<Message> Message::decode(Connection& conn) {
    std::string output = conn.read_raw(3);
    MessageType const type = (MessageType) std::stoi(output);
    switch (type) {
    case MT_CLIENT_LOG_IN: return std::make_unique<MClientLogIn>(conn);
    case MT_CLIENT_UPDATE: return std::make_unique<MClientUpdate>(conn);
    case MT_CLIENT_FINISH: throw std::exception();
    case MT_CLIENT_EXIT:   throw std::exception();
    case MT_SERVER_UPDATE: return std::make_unique<MServerUpdate>(conn);

    // TODO: exceptions
    default:
        throw std::runtime_error("Invalid code");
    }
}

Message::~Message() { }

static std::string pad_number(int const num, int const digits) {
    std::ostringstream ostr;
    ostr << std::setw(digits) << std::setfill('0') << num;
    return ostr.str();
}


MClientLogIn::MClientLogIn(std::string const& username) : _username(username) {}

MClientLogIn::MClientLogIn(Connection& conn) {
    unsigned const len_username = std::stoi(conn.read_raw(2));
    this->_username = conn.read_raw(len_username);
}

std::string MClientLogIn::encode() const {
    std::string output = "";
    output += std::to_string(MT_CLIENT_LOG_IN); // code
    output += pad_number(this->_username.size(), 2); // username length
    output += this->_username; // the username itself
    return output;
}



MClientUpdate::MClientUpdate(
    std::string const& recipient,
    std::string const& message
) : _recipient(recipient), _message(message)
{ }

MClientUpdate::MClientUpdate(Connection& conn) {
    unsigned const len_recipient = std::stoi(conn.read_raw(2));
    this->_recipient = conn.read_raw(len_recipient);
    unsigned const len_message = std::stoi(conn.read_raw(5));
    this->_message = conn.read_raw(len_message);
}

std::string MClientUpdate::encode() const {
    std::string output = "";
    output += std::to_string(MT_CLIENT_UPDATE); // code
    output += pad_number(this->_recipient.size(), 2); // recipient length
    output += this->_recipient; // the recipient itself
    output += pad_number(this->_message.size(), 5); // the message length
    output += this->_recipient;
    return output;
}



MServerUpdate::MServerUpdate(
    std::string const& chat_content,
    std::string const& recipient,
    std::set<std::string> const& _connected_users
) : _chat_content(chat_content)
  , _recipient(recipient)
  , _connected_users(_connected_users)
{ }

MServerUpdate::MServerUpdate(Connection& conn) {
    unsigned const len_chat_content = std::stoi(conn.read_raw(5));
    this->_chat_content = conn.read_raw(len_chat_content);
    unsigned const len_recipient = std::stoi(conn.read_raw(2));
    this->_recipient = conn.read_raw(len_recipient);

    unsigned const len_all_users = std::stoi(conn.read_raw(5));
    std::string const all_users = conn.read_raw(len_all_users);

    decltype(all_users)::size_type start = 0;
    while (start < all_users.size()) {
        decltype(all_users)::size_type end = all_users.find('&', start);
        if (end == decltype(all_users)::npos)
            end = all_users.size();

        std::string const username = all_users.substr(start, end - start);
        this->_connected_users.insert(username);
        start = end + 1;
    }
}

std::string MServerUpdate::encode() const {
    std::string output = "";
    output += std::to_string(MT_SERVER_UPDATE); // code
    output += pad_number(this->_chat_content.size(), 5); // chat content length
    output += this->_chat_content; // the chat content itself
    output += pad_number(this->_recipient.size(), 2); // recipient length
    output += this->_recipient; // the recipient itself

    bool first = true;
    std::string all_users = 0;
    for (auto const& username : this->_connected_users) {
        if (first) {
            first = false;
        } else {
            all_users.push_back('&');
        }

        all_users += username;
    }

    return output;
}

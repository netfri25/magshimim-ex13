#include "Connection.h"
#include "Chat.h"

namespace std {

std::string to_string(ChatMessage const& msg) {
    std::string output = "";
    output += MESSAGE_MAGIC;
    output += AUTHOR_MAGIC;
    output += msg.author;
    output += DATA_MAGIC;
    output += msg.data;
    return output;
}

}

ChatMessage::ChatMessage(
    std::string const& author,
    std::string const& recipient,
    std::string const& data
)
    : author(author)
    , recipient(recipient)
    , data(data)
{ }

// Chat::Chat(std::vector<ChatMessage> const& msgs) : _msgs(msgs) {}

// std::ostream& operator<<(std::ostream& os, Chat const& chat) {
//     for (auto const& msg : chat._msgs)
//         os << msg.author << ": " << msg.data << std::endl;

//     return os;
// }

#pragma once

#include <ostream>
#include <string>
#include <vector>

struct ChatMessage {
    std::string author;
    std::string recipient;
    std::string data;

    ChatMessage(
        std::string const& author,
        std::string const& recipient,
        std::string const& data
    );
};

namespace std {
std::string to_string(ChatMessage const&);
}

// class Chat {
// public:
//     Chat() = default;
//     Chat(std::vector<ChatMessage> const& msgs);
//     friend std::ostream& operator<<(std::ostream&, Chat const&);
// private:
//     std::vector<ChatMessage> _msgs; // stupid idea, but it works
// };

#ifndef CLIENT_HPP
#define CLIENT_HPP

# include <string>

class Client {
    public:
        int fd;
        std::string bufferBites;

    bool validPass;
    std::string nickName;
    std::string userName;
    std::string realName;

    Client();
    bool isClientAuth() const;
};

#endif
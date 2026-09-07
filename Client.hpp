#ifndef CLIENT_HPP
#define CLIENT_HPP


# include <string>

class Client {
    public:
        int fd;
        std::string bufferBites;
};

#endif
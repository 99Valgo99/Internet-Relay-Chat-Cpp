# ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <sys/socket.h>
# include <fcntl.h>
# include <string>
# include <poll.h>
# include <cstdlib>
# include <vector>
# include <cstring>
# include <netinet/in.h> // sockaddr_in
# include <arpa/inet.h> // INADDR_ANY/htons
# include <map>
# include <unistd.h>
# include <sstream>

# include "Client.hpp"
# include "Channel.hpp"

class Server {
    private:
        int listen_fd;
        std::vector<struct pollfd> poll_fds;
        std::map<int, Client> clients;
        std::string password;
        std::map<std::string, Channel> channels;
    
        void setupSocket(int port);
        void acceptNclient();
        bool handleClientData(int fd);
        void validatePass(int fd, std::string arg);
        void validateNick(int fd, std::string arg);
        void validateUser(int fd, std::string username, std::string realname);

    public:
        Server(int port, std::string _password);
        void run();
};

# endif
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
        std::string password;
        std::map<int, Client> clients;
        std::vector<struct pollfd> poll_fds;
        std::map<std::string, Channel> channels;
    
        void acceptNclient();
        bool spreadMessage(int fd);
        void setupSocket(int port);
        void validatePass(int fd, std::string arg);
        void validateNick(int fd, std::string arg);
        void validateUser(int fd, std::string username, std::string realname);
        
        bool handleClientData(int fd);
        void handleJoin(int fd, std::string nameChannel);
        void handlePrvMsg(int fd, std::string targets, std::string message);

        void msgSendToNick(std::string target, std::string message);
        void msgSendToChannel(int fd, std::string target, std::string message);
    
    public:
        Server(int port, std::string _password);
        void run();
};

# endif
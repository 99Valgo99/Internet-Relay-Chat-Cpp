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
# include <climits>

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
        
        void exitAllChannels(int fd);
        bool handleClientData(int fd);
        void handleJoin(int fd, std::string nameChannel, std::string key);
        void handleTopic(int fd, std::string channelT, std::string _topic);
        void handlePrvMsg(int fd, std::string targets, std::string message);
        void handleInvite(int fd, std::string _nickname, std::string _channel);
        void kickOneClient(int fd, std::string listChannel, std::string listUser);
        void handleKick(int fd, std::vector<std::string> listChannel, std::vector<std::string> listUsers, std::string comment);
        void handleMode(int fd, std::string channel_, char sign, char flag, std::string arg);
        bool userLimitHelper(Channel& _Channel, std::string arg);
        void operatorModeHelper(Channel& _Channel, bool& toggle, std::string& arg);

        std::string buildSenderPrifix(int fd);
        void msgSendToNick(int fd, std::string target, std::string message);
        void msgSendToChannel(int fd, std::string target, std::string message);

        bool needAnArg(char sign, char flag);
        bool getTheArg(std::vector<std::string>& argLeft, size_t& index, std::string& consumedArg);

        void sendServerReply(int fd, int code, std::string message);
    
    public:
        Server(int port, std::string _password);
        void run();
};

# endif
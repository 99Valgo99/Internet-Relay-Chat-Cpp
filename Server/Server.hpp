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

# include "../Client/Client.hpp"
# include "../Channel/Channel.hpp"

class Server {

    private:
        int listen_fd;
        std::string password;
        std::map<int, Client> clients;
        std::vector<struct pollfd> poll_fds;
        std::map<std::string, Channel> channels;
    
        // server loop helpers
        void acceptNclient();
        bool spreadMessage(int fd);
        void setupSocket(int port);
        
        // validators
        void validatePass(int fd, std::string arg);
        void validateNick(int fd, std::string arg);
        void validateUser(int fd, std::string username, std::string realname);
        
        // command handlers
        void exitAllChannels(int fd);
        bool handleClientData(int fd);
        bool userLimitHelper(Channel& _Channel, std::string arg);
        void handleJoin(int fd, std::string nameChannel, std::string key);
        void handleTopic(int fd, std::string channelT, std::string _topic);
        void handlePrvMsg(int fd, std::string targets, std::string message);
        void handleInvite(int fd, std::string _nickname, std::string _channel);
        void kickOneClient(int fd, std::string listChannel, std::string listUser);
        void operatorModeHelper(int fd, Channel& _Channel, bool& toggle, std::string& arg);
        void handleMode(int fd, std::string channel_, char sign, char flag, std::string arg);
        void handleKick(int fd, std::vector<std::string> listChannel, std::vector<std::string> listUsers, std::string comment);

        // send + broadcast
        void sendWelcome(int fd);
        std::string buildSenderPrifix(int fd);
        void sendServerReply(int fd, int code, std::string message);
        void msgSendToChannel(int fd, std::string target, std::string message);
        void msgSendToNick(int fd, std::string target, std::string message);
        void sendChangeNick(int fd, std::string arg, std::string oldNickname);

        // mode helpers
        bool needAnArg(char sign, char flag);
        bool getTheArg(std::vector<std::string>& argLeft, size_t& index, std::string& consumedArg);


        // dispatchers
        void dispatchPass(int fd, std::istringstream& stream);
        void dispatchNick(int fd, std::istringstream& stream);
        void dispatchUser(int fd, std::istringstream& stream);
        void dispatchJoin(int fd, std::istringstream& stream);
        void dispatchPrvMsg(int fd, std::istringstream& steam);
        void dispatchKick(int fd, std::istringstream& stream);
        void dispatchTopic(int fd, std::istringstream& stream);
        void dispatchInvite(int fd, std::istringstream& stream);
        void dispatchMode(int fd, std::istringstream& stream);
    
    public:
        Server(int port, std::string _password);
        void run();
};

# endif
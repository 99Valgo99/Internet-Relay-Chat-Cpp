#include "Server.hpp"

Server::Server(int port, std::string _password)
{
    this->password = _password;
    setupSocket(port);
}

void Server::setupSocket(int port)
{
    this->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
        throw std::runtime_error("Error: Cannot Create Socket");

    int opt = 1;
    if (setsockopt(this->listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("Error");

    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    // address.sin_addr.s_addr = INADDR_ANY;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (bind(this->listen_fd, (sockaddr*)&address, sizeof(address)))
        throw std::runtime_error("Error");
    if (listen(this->listen_fd, SOMAXCONN))
        throw std::runtime_error("Error");
    if (fcntl(this->listen_fd, F_SETFL, O_NONBLOCK))
        throw std::runtime_error("Error");

    struct pollfd pfd;
    pfd.fd = this->listen_fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    this->poll_fds.push_back(pfd);
}

void Server::run()
{
    while (true)
    {
        for (size_t i = 0; i < poll_fds.size(); i++)
        {
            if (poll_fds[i].fd == this->listen_fd)
                continue ;
            if (!clients[poll_fds[i].fd].sendBytes.empty())
                poll_fds[i].events = POLLIN | POLLOUT;
            else
                poll_fds[i].events = POLLIN;
        }
        int up = poll(&this->poll_fds[0], poll_fds.size(), -1);
        if (up == -1)
        {
            std::cerr << "Internal Server Error: poll() Failed !" << std::endl;
            continue ;
        }
        std::vector<int> needRemove;
        for (size_t i = 0; i < this->poll_fds.size(); i++)
        {
            if (this->poll_fds[i].revents & POLLIN)
            {
                if (poll_fds[i].fd == this->listen_fd)
                    acceptNclient();
                else
                {
                    bool present = handleClientData(this->poll_fds[i].fd);
                    if (!present)
                        needRemove.push_back(poll_fds[i].fd);
                }
            }
            if (this->poll_fds[i].revents & POLLOUT)
            {
                bool alive = spreadMessage(poll_fds[i].fd);
                if (!alive)
                    needRemove.push_back(poll_fds[i].fd);
            }
        }
        for (size_t j = 0; j < needRemove.size(); j++)
        {
            int deadFd = needRemove[j];
            for (size_t x = 0; x < this->poll_fds.size(); x++)
            {
                if (this->poll_fds[x].fd == deadFd)
                {
                    this->poll_fds.erase(this->poll_fds.begin() + x);
                    break;
                }
            }
            clients.erase(deadFd);
        }
    }
}

void Server::acceptNclient()
{
    Client cl;
    cl.fd = accept(this->listen_fd, NULL, NULL);
    if (cl.fd == -1)
    {
        std::cerr << "Internal Server Error: accept() failed !" << std::endl;
        return ;
    }
    struct pollfd pfd_client;
    pfd_client.fd = cl.fd;
    pfd_client.events = POLLIN;
    pfd_client.revents = 0;

    this->poll_fds.push_back(pfd_client);
    std::cout << "New Client [fd]: " << cl.fd << " Is In..." << std::endl;
    this->clients.insert(std::make_pair(cl.fd, cl));
}

bool Server::handleClientData(int fd)
{
    char tempo[1024];
    int bytes = recv(fd, tempo, sizeof(tempo), 0);
    if (bytes > 0) {
        clients[fd].bufferBites.append(tempo, bytes);

        while (true)
        {
            size_t pos = clients[fd].bufferBites.find('\n');
            if (pos == std::string::npos)
                break ;
            std::string wellFormed = clients[fd].bufferBites.substr(0, pos);
            clients[fd].bufferBites.erase(0, pos + 1);

            if (!wellFormed.empty() && wellFormed[wellFormed.size() - 1] == '\r')
                wellFormed.erase(wellFormed.size() - 1);
            
            std::istringstream stream(wellFormed);
            std::string command;
            stream >> command;
            if (command == "PASS")
            {
                std::string arg;
                std::getline(stream, arg);

                if (arg.empty())
                {
                    sendServerReply(fd, 461, "Error: PASS should have one argument at least !");
                    continue ;
                }
                if (!arg.empty() && arg[0] == ' ')
                    arg.erase(0, 1);
                if (!arg.empty() && arg[0] == ':')
                {
                    arg.erase(0, 1);
                    validatePass(fd, arg);
                }
                else
                {
                    if (arg.find(' ') != std::string::npos)
                    {
                        sendServerReply(fd, 464, "Error: Password Mismatch !");
                        continue ;
                    }
                    validatePass(fd, arg);
                }
            }
            else if (command == "NICK")
            {
                std::string arg, leftovers;
                stream >> arg;
                if (stream >> leftovers)
                    sendServerReply(fd, 461, "Error: NICK Should Have one argumet !");
                else
                    validateNick(fd, arg);
            }
            else if (command == "USER")
            {
                std::string username, mode, unused, realname;
                stream >> username >> mode >> unused;

                if (username.empty() || mode.empty() || unused.empty())
                {
                    sendServerReply(fd, 461, "Error: User needs more parameters !");
                    continue ;
                }
                std::getline(stream, realname);
                if (!realname.empty() && realname[0] == ' ')
                    realname.erase(0, 1);
                if (realname.empty() || realname[0] != ':')
                {
                    sendServerReply(fd, 461, "Error: USER realname should start with ':' !");
                    continue ;
                }
                else
                {
                    realname.erase(0, 1);
                    validateUser(fd, username, realname);
                }
            }
            else if (command == "JOIN")
            {
                std::string channelname, keys, leftovers;
                stream >> channelname >> keys;
                if (channelname.empty())
                {
                    sendServerReply(fd, 461, "Error: JOIN Channel must have a name !");
                    continue ;
                }
                if (channelname == "0")
                {
                    exitAllChannels(fd);
                    continue ;
                }
                if (stream >> leftovers)
                {
                    sendServerReply(fd, 999, "Error: JOIN should have one argument !");
                    continue ;
                }
                std::vector<std::string> keyholder;
                size_t start = 0;
                while (true)
                {
                    std::string oneKey;
                    size_t posComma = keys.find(',', start);
                    if (posComma == std::string::npos)
                    {
                        oneKey = keys.substr(start);
                        keyholder.push_back(oneKey);
                        break ;
                    }
                    oneKey = keys.substr(start, posComma - start);
                    keyholder.push_back(oneKey);
                    start = posComma + 1;
                }
                start = 0;
                size_t key_index = 0;
                std::string emptyKey;
                while (true)
                {
                    std::string getOneChannel;
                    size_t posComma = channelname.find(',', start);
                    if (posComma == std::string::npos)
                    {
                        getOneChannel = channelname.substr(start);
                        if (key_index >= keyholder.size())
                        {
                            handleJoin(fd, getOneChannel, emptyKey);
                            break ;
                        }
                        else
                        {
                            handleJoin(fd, getOneChannel, keyholder[key_index]);
                            break ;
                        }
                    }
                    getOneChannel = channelname.substr(start, posComma - start);
                    if (key_index > keyholder.size())
                        handleJoin(fd, getOneChannel, emptyKey);
                    else
                        handleJoin(fd, getOneChannel, keyholder[key_index]);
                    start = posComma + 1;
                    key_index++;
                }
            }
            else if (command == "PRIVMSG")
            {
                std::string targets, message;
                stream >> targets;
                if (targets.empty())
                {
                    sendServerReply(fd, 461, "Error: PRIVMSG Needs more parameters !");
                    continue ;
                }
                std::getline(stream, message);
                if (!message.empty() && message[0] == ' ')
                    message.erase(0, 1);
                if (message.empty() || message[0] != ':')
                {
                    sendServerReply(fd, 412, "Error: PRIVMSG second arg format [:msg] !");
                    continue ;
                }
                handlePrvMsg(fd, targets, message);
            }
            else if (command == "KICK")
            {
                std::string channelList, usersList, comment;
                stream >> channelList >> usersList;
                if (channelList.empty() || usersList.empty())
                {
                    sendServerReply(fd, 461, "Error: KICK needs a channel + users to kick !");
                    continue ;
                }
                std::getline(stream, comment);
                if (!comment.empty() && comment[0] == ' ')
                    comment.erase(0, 1);
                if (!comment.empty())
                {
                    if (comment[0] == ':')
                        comment.erase(0, 1);
                    else
                    {
                        sendServerReply(fd, 461, "Error: KICK Comment arg should start with ':' !");
                        continue ;
                    }
                }
                size_t start = 0;
                std::vector<std::string> listChannel, listUsers;
                while (true)
                {
                    std::string target;
                    size_t posComma = channelList.find(',', start);
                    if (posComma == std::string::npos)
                    {
                        target = channelList.substr(start);
                        listChannel.push_back(target);
                        break ;
                    }
                    target = channelList.substr(start, posComma - start);
                    listChannel.push_back(target);
                    start = posComma + 1;
                }
                start = 0;
                while (true)
                {
                    std::string target;
                    size_t posComma = usersList.find(',', start);
                    if (posComma == std::string::npos)
                    {
                        target = usersList.substr(start);
                        listUsers.push_back(target);
                        break ;
                    }
                    target = usersList.substr(start, posComma - start);
                    listUsers.push_back(target);
                    start = posComma + 1;
                }
                handleKick(fd, listChannel, listUsers, comment);
            }
            else if (command == "TOPIC")
            {
                std::string channel, topic;
                stream >> channel;
                if (channel.empty())
                {
                    sendServerReply(fd, 461, "Error: TOPIC must have at least one argument !");
                    continue ;
                }
                std::getline(stream, topic);
                if (!topic.empty() && topic[0] == ' ')
                    topic.erase(0, 1);
                handleTopic(fd, channel, topic);
            }
            else if (command == "INVITE")
            {
                std::string nickname, channel, leftovers;
                stream >> nickname >> channel;
                if (nickname.empty() || channel.empty())
                {
                    sendServerReply(fd, 461, "Error: INVITE Malformed input !");
                    continue ;
                }
                std::getline(stream, leftovers);
                if (!leftovers.empty() && leftovers[0] == ' ')
                    leftovers.erase(0, 1);
                if (!leftovers.empty())
                {
                    sendServerReply(fd, 999, "Error: INVITE requires only two args <nickname> and <channel> !");
                    continue ;
                }
                handleInvite(fd, nickname, channel);
            }
            else if (command == "MODE")
            {
                char sign;
                size_t threeArgCounter = 0;
                std::string modeStr, channelModed;
                stream >> channelModed >> modeStr;

                if (!channelModed.empty() && channelModed[0] != '#')
                {
                    sendServerReply(fd, 461, "Error: MODE Needs more arguments !");
                    continue ;
                }
                std::string oneArgVector;
                std::vector<std::string> argsLeft;

                while (stream >> oneArgVector)
                    argsLeft.push_back(oneArgVector);
                for (size_t i = 0; i < modeStr.size(); i++)
                {
                    if (modeStr[i] == '-' || modeStr[i] == '+')
                    {
                        sign = modeStr[i];
                        std::cout << "Sign Currently is: " << sign << std::endl;
                    }
                    else
                    {
                        if (modeStr[i] == 'i' || modeStr[i] == 'k' || modeStr[i] == 'l'
                            || modeStr[i] == 'o' || modeStr[i] == 't')
                        {
                            std::cout << "Valid Flag: " << modeStr[i] << std::endl;
                            bool needsArgBoolean = needAnArg(sign, modeStr[i]);
                            if (needsArgBoolean)
                            {
                                if (threeArgCounter + 1 > 3)
                                {
                                    sendServerReply(fd, 999, "Error: MODE excess in use of flags that need args !");
                                    continue ;
                                }
                                bool argConsumption = getTheArg(argsLeft, threeArgCounter, oneArgVector);
                                if (!argConsumption)
                                {
                                    sendServerReply(fd, 461, "Error: MODE Malformed input !");
                                    continue ;
                                }
                                handleMode(fd, channelModed, sign, modeStr[i], oneArgVector);
                            }
                            else
                            {
                                std::string empty_arg;
                                handleMode(fd, channelModed, sign, modeStr[i], empty_arg);
                            }
                        }
                        else
                            sendServerReply(fd, 472, "Error: Mode Unknown Mode !");
                    }
                }
            }
            else
                sendServerReply(fd, 421, "Error: Unkown Command !");
        }
        return true;
    }
    else if (bytes == 0)
    { // client is dead.
        close(fd);
        return false;
    }
    else
    { // bytes < 0 might be recv error or poll() already said readable.
        close(fd);
        return false;
    }
}

bool Server::needAnArg(char sign, char flag)
{
    if (flag == 'i' && (sign == '-' || sign == '+'))
        return false;

    else if (flag == 't' && (sign == '-' || sign == '+'))
        return false;

    else if (flag == 'l' && sign == '-')
        return false;

    else if (flag == 'l' && sign == '+')
        return true;

    else if (flag == 'k' && (sign == '-' || sign == '+'))
        return true;

    else if (flag == 'o' && (sign == '-' || sign == '+'))
        return true;

    return false;
}

bool Server::getTheArg(std::vector<std::string>& argLeft, size_t& index, std::string& consumedArg)
{
    if (index >= argLeft.size()) // the case of consumed arg while still there is a flag [+ok Someone, ok gets someone, k needs an arg while index match the size of the vector]
        return false;
    consumedArg = argLeft[index];
    std::cout << "Arg consumed: " << consumedArg << std::endl;
    index++;
    return true;
}

void Server::sendServerReply(int fd, int code, std::string message)
{
    std::map<int, Client>::iterator it_client = this->clients.find(fd);
    std::ostringstream reply;
    reply << "ircserv " << code << " " << it_client->second.nickName << " :" << message;
    it_client->second.sendBytes.append(reply.str() + "\r\n");
}
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
    address.sin_addr.s_addr = INADDR_ANY;

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
        int up = poll(&this->poll_fds[0], poll_fds.size(), -1);
        if (up == -1)
        {
            std::cerr << "Error" << std::endl;
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
        std::cerr << "Error" << std::endl;
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
                std::string arg, leftovers;
                stream >> arg;
                if (stream >> leftovers)
                    std::cerr << "Error: PASS should have one argument" << std::endl;
                else
                    validatePass(fd, arg);
            }
            else if (command == "NICK")
            {
                std::string arg, leftovers;
                stream >> arg;
                if (stream >> leftovers)
                    std::cerr << "Error: NICK should have one argument" << std::endl;
                else
                    validateNick(fd, arg);
            }
            else if (command == "USER")
            {
                std::string username, mode, unused, realname;
                stream >> username >> mode >> unused;

                if (username.empty() || mode.empty() || unused.empty())
                {
                    std::cerr << "Error: USER command requires [username, mode, unused, realname]" << std::endl;
                    continue ;
                }
                std::getline(stream, realname);
                if (!realname.empty() && realname[0] == ' ')
                    realname.erase(0, 1);
                if (realname.empty() || realname[0] != ':')
                {
                    std::cerr << "Error: USER realname argument should start with ':'" << std::endl;
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
                std::string channelname, leftovers;
                stream >> channelname;
                if (stream >> leftovers)
                    std::cerr << "Error: JOIN should have one argument" << std::endl;
                else
                    handleJoin(fd, channelname);
            }
            else
            {
                std::cerr << "Error: Unrecognized Command" << std::endl;
                // for now, later i will see into adding Error Codes.
            }

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

void Server::validatePass(int fd, std::string arg)
{
    if (arg == this->password)
    {
        clients[fd].validPass = true;
        std::cout << "Passowrd Confirmed !" << std::endl;
    }
    else
    {
        std::cerr << "Error: Wrong Password" << std::endl;
        return ; // added
        // to develop more...
    }
}

void Server::validateUser(int fd, std::string username, std::string realname)
{
    if (!clients[fd].validPass)
    {
        std::cerr << "Error, Needs a password before using USER: PASS ****" << std::endl;
        return ;
    }
    if (username.empty())
    {
        std::cerr << "No Username Provided, Expected Format: USER username mode unused :realname" << std::endl;
        return ;
    }
    clients[fd].userName = username;
    clients[fd].realName = realname;
    std::cout << "Confirmed Username: " << clients[fd].userName << std::endl;
    std::cout << "Confirmed realname: " << clients[fd].realName << std::endl;
}

void Server::validateNick(int fd, std::string arg)
{
    if (!clients[fd].validPass)
    {
        std::cerr << "Error, Needs a password before using NICK: PASS ****" << std::endl;
        return ;
    }
    if (arg.empty())
    {
        std::cerr << "No Nickname Provided, Expected Format: NICK nickname" << std::endl;
        return ;
    }
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->first == fd)
            continue ;
        if (it->second.nickName == arg)
        {
            std::cerr << "Sorry ! Nickname Is Already Taken, Chose Something Else" << std::endl;
            return ;
        }
    }
    clients[fd].nickName = arg;
    std::cout << "Confirmed Nickname: " << clients[fd].nickName << std::endl;
}

void Server::handleJoin(int fd, std::string nameChannel)
{
    if (!clients[fd].isClientAuth())
    {
        std::cerr << "Error: You are not authenticated yet !" << std::endl;
        return ;
    }
    std::map<std::string, Channel>::iterator it = channels.find(nameChannel);
    if (it == channels.end())
    {
        Channel newChannel(nameChannel, fd);
        this->channels.insert(std::make_pair(nameChannel, newChannel));
        std::cout << "Channel was created and client was added to channel: " << nameChannel << std::endl;
    }
    else
    {
        it->second.addClientsToChannel(fd);
        std::cout << "Client was added to the existing channel: " << nameChannel << std::endl;
    }
}
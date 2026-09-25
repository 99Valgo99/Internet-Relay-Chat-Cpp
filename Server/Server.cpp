#include "Server.hpp"

volatile int Server::signalComing = 0;

void Server::signalHandler(int signumber)
{
    (void)signumber;
    signalComing = 1;
}

Server::Server(int port, std::string _password)
{
    this->password = _password;
    setupSocket(port);
}

void Server::setupSocket(int port)
{
    this->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
        throw std::runtime_error("Internal Server Error: Cannot Create Socket");

    int opt = 1;
    if (setsockopt(this->listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("Internal Server Error");

    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(this->listen_fd, (sockaddr*)&address, sizeof(address)))
        throw std::runtime_error("Internal Server Error");
    if (listen(this->listen_fd, SOMAXCONN))
        throw std::runtime_error("Internal Server Error");
    if (fcntl(this->listen_fd, F_SETFL, O_NONBLOCK))
        throw std::runtime_error("Internal Server Error");

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
        if (Server::signalComing == 1)
        {
            this->cleanUp();
            break;
        }
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
            exitAllChannels(deadFd);
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
    if (fcntl(cl.fd, F_SETFL, O_NONBLOCK) == -1)
    {
        close(cl.fd);
        throw std::runtime_error("Internal Server Error");
    }
    struct pollfd pfd_client;
    pfd_client.fd = cl.fd;
    pfd_client.events = POLLIN;
    pfd_client.revents = 0;

    this->poll_fds.push_back(pfd_client);
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

            if (command.empty())
                continue ;
            if (command == "PASS")
                dispatchPass(fd, stream);
            else if (command == "NICK")
                dispatchNick(fd, stream);
            else if (command == "USER")
                dispatchUser(fd, stream);
            else if (command == "JOIN")
                dispatchJoin(fd, stream);
            else if (command == "PRIVMSG")
                dispatchPrvMsg(fd, stream);
            else if (command == "KICK")
                dispatchKick(fd, stream);
            else if (command == "TOPIC")
                dispatchTopic(fd, stream);
            else if (command == "INVITE")
                dispatchInvite(fd, stream);
            else if (command == "MODE")
                dispatchMode(fd, stream);
            else if (command == "PING")
            {
                std::string token;
                std::getline(stream, token);
                if (!token.empty() && token[0] == ' ')
                    token.erase(0, 1);
                clients[fd].sendBytes.append("PONG " + token + "\r\n");
            }
            else
                sendServerReplyarg(fd, 421, command, "Unkown Command !");
        }
        return true;
    }
    else if (bytes == 0)
    {
        close(fd);
        return false;
    }
    else
    {
        close(fd);
        return false;
    }
}

void Server::cleanUp()
{
    close(this->listen_fd);
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
        close (it->first);
}

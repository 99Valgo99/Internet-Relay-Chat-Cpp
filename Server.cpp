#include "Server.hpp"

Server::Server(int port, std::string _password) {
    this->password = _password;
    setupSocket(port);
}

void Server::setupSocket(int port) {

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

void Server::run() {

    while (true) {

        int up = poll(&this->poll_fds[0], poll_fds.size(), -1);
        if (up == -1) {
            std::cerr << "Error" << std::endl;
            continue ;
        }

        std::vector<int> needRemove;
        for (size_t i = 0; i < this->poll_fds.size(); i++) {
            if (this->poll_fds[i].revents & POLLIN) {
                if (poll_fds[i].fd == this->listen_fd)
                    acceptNclient();
                else {
                    bool present = handleClientData(this->poll_fds[i].fd);
                    if (!present)
                        needRemove.push_back(poll_fds[i].fd);
                }
            }
        }
        for (size_t j = 0; j < needRemove.size(); j++) {
            int deadFd = needRemove[j];

            for (size_t x = 0; x < this->poll_fds.size(); x++) {
                if (this->poll_fds[x].fd == deadFd) {
                    this->poll_fds.erase(this->poll_fds.begin() + x);
                    break;
                }
            }
            clients.erase(deadFd);
        }
    }
}

void Server::acceptNclient() {
    Client cl;
    cl.fd = accept(this->listen_fd, NULL, NULL);
    if (cl.fd == -1) {
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

bool Server::handleClientData(int fd) {
    char tempo[1024];
    int bytes = recv(fd, tempo, sizeof(tempo), 0);
    if (bytes > 0) {
        clients[fd].bufferBites.append(tempo, bytes);

        while (true) {
            size_t pos = clients[fd].bufferBites.find('\n');
            if (pos == std::string::npos)
                break ;
            std::string wellFormed = clients[fd].bufferBites.substr(0, pos);
            clients[fd].bufferBites.erase(0, pos + 1);

            if (!wellFormed.empty() && wellFormed[wellFormed.size() - 1] == '\r')
                wellFormed.erase(wellFormed.size() - 1);
                // TODO 
        }
        return true;
    } else if (bytes == 0) { // client is dead.
        close(fd);
        return false;
    } else { // bytes < 0 might be recv error or poll() already said readable.
        close(fd);
        return false;
    }
}
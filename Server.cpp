# include <iostream>
# include <sys/socket.h>
# include <fcntl.h>
# include <string>
# include <poll.h>
# include <loop>

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Error: Expected Format ./ircserv <port> <password>" << std::endl;
        exit(1);
    }
    int port = std::atoi(argv[1]);
    std::string password = argv[2];

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        std::cerr << "Error: Cannot Create Socket" << std::endl;
        exit(1);
    }
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "Error" << std::endl;
        exit(1);
    }
    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_fd, (sockaddr*)&address, sizeof(address))) {
        std::cerr << "Error" << std::endl;
        exit(1);
    }
    if (listen(listen_fd, SOMAXCONN)) {
        std::cerr << "Error" << std::endl;
        exit(1);
    }
    if (fcntl(listen_fd, F_SETFL, O_NONBLOCK)) {
        std::cerr << "Error" << std::endl;
        exit(1);
    }
    struct pollfd pfd;
    pfd.fd = listen_fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    std::vector<struct pollfd> poll_fds;
    poll_fds.push_back(pfd);
}
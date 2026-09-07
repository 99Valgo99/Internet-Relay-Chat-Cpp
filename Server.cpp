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

    while (true) {
        int up = poll(&poll_fds[0], poll_fds.size(), -1);
        if (up == -1) {
            std::cerr << "poll Error" << std::endl;
            continue ;
        }
        for (size_t i = 0; i < poll_fds.size(); i++) {
            if (poll_fds[i].revents & POLLIN) {
                if (poll_fds[i].fd == listen_fd) {
                    int client_fd = accept(listen_fd, NULL, NULL);
                    if (client_fd == -1) {
                        std::cerr << "Error" << std::endl;
                        continue ;
                    }
                    fcntl(client_fd, F_SETFL, O_NONBLOCK);
                    struct pollfd pfd_client;
                    pfd_client.fd = client_fd;
                    pfd_client.events = POLLIN;
                    pfd_client.revents = 0;
                    poll_fds.push_back(pfd_client);
                    std::cout << "New Client [fd]: " << client_fd << " Is In..." << std::endl;
                }
            }
        }
    }
}
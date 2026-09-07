# include "Server.hpp"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Error: Expected Format ./ircserv <port> <password>" << std::endl;
        exit(1);
    }
    try {
        Server Serv(std::atoi(argv[1]), argv[2]);
        Serv.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
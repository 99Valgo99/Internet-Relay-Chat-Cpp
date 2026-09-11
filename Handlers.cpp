# include "Server.hpp"

void Server::handleJoin(int fd, std::string nameChannel)
{
    if (!clients[fd].isClientAuth())
    {
        std::cerr << "Error: You are not authenticated yet !" << std::endl;
        return ;
    }
    if (nameChannel.empty() || nameChannel[0] != '#')
    {
        std::cerr << "Error: A channel should always start with '#' !" << std::endl;
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

void Server::handlePrvMsg(int fd, std::string targets, std::string message)
{
    (void)fd, (void)targets, (void)message;
    std::cout << "PRVMSG was succesfully called !" << std::endl;
}
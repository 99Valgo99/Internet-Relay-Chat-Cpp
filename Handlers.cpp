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
    std::cout << "PRVMSG was succesfully called !" << std::endl;
    if (!clients[fd].isClientAuth())
    {
        std::cerr << "Error: You are not authenticated yet !" << std::endl;
        return ;
    }
    
    message.erase(0, 1);
    size_t start = 0;
    bool lastOne = false;
    while (!lastOne)
    {
        std::string oneTarget;
        size_t posComma = targets.find(',', start);
        if (posComma == std::string::npos)
        {
            oneTarget = targets.substr(start);
            lastOne = true;
        }
        else
        {
            oneTarget = targets.substr(start, posComma - start);
            start = posComma + 1;
        }
        if (!oneTarget.empty() && oneTarget[0] == '#')
            msgSendToChannel(fd, oneTarget, message);
        else
            msgSendToNick(fd, oneTarget, message);
    }
}
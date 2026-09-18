# include "../Server/Server.hpp"

std::string Server::buildSenderPrifix(int fd)
{
    std::string Sender = ":" + this->clients[fd].nickName + "!" + this->clients[fd].userName + "@localhost";
    return Sender;
}

void Server::msgSendToNick(int fd, std::string target, std::string message)
{
    if (target.empty())
    {
        // std::cerr << "ERROR: PRIVMSG does not accept empty target !" << std::endl;
        sendServerReply(fd, 461, "ERROR: PRIVMSG needs more parameters !");
        return ;
    }
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second.nickName == target)
        {
            std::cout << "Sending to Client: (updated output) -> " << it->second.nickName << std::endl;
            std::string sender = buildSenderPrifix(fd);
            it->second.sendBytes.append(sender + " PRIVMSG " + target + " :" + message + "\r\n");
            return ;
        }
    }
    // std::cout << "Error: No such a client with the nickname:" << target << std::endl;
    sendServerReply(fd, 401, "Error: No such a client with the nickname:");
}

void Server::msgSendToChannel(int fd, std::string target, std::string message)
{
    std::cout << "Broadcasting to channel: " << target << std::endl;
    if (target.empty())
    {
        // std::cerr << "Error: PRIVMSG does not accept empty target !" << std::endl;
        sendServerReply(fd, 461, "ERROR: PRIVMSG needs more parameters !");
        return ;
    }
    std::map<std::string, Channel>::iterator it = this->channels.find(target);
    if (it == this->channels.end())
    {
        // std::cerr << "Error: No Channel was found with this name !" << std::endl;
        sendServerReply(fd, 404, "Error: No Channel was found with this name !");
        return ;
    }
    else
    {
        std::string sender = buildSenderPrifix(fd);
        const std::set<int>& members = it->second.getChannelsMembers();
        if (members.find(fd) == members.end())
        {
            // std::cerr << "Error: The client has not joined this channel !" << std::endl;
            sendServerReply(fd, 404, "Error: The client has not joined this channel !");
            return ;
        }
        for (std::set<int>::const_iterator it_members = members.begin(); it_members != members.end(); ++it_members)
        {
            if (*it_members == fd)
                continue ;
            this->clients[*it_members].sendBytes.append(sender + " PRIVMSG " + target + " :" + message + "\r\n");
        }
    }
}

bool Server::spreadMessage(int fd)
{
    int sendResult = send(fd, clients[fd].sendBytes.c_str(), clients[fd].sendBytes.size(), 0);
    if (sendResult == -1)
    {
        close(fd);
        return false;
    }
    clients[fd].sendBytes.erase(0, sendResult);
    return true;
}
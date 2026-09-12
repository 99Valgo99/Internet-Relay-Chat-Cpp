# include "Server.hpp"

std::string Server::buildSenderPrifix(int fd)
{
    std::string Sender = ":" + this->clients[fd].nickName + "!" + this->clients[fd].userName + "@localhost";
    return Sender;
}

void Server::msgSendToNick(int fd, std::string target, std::string message)
{
    if (target.empty())
    {
        std::cerr << "ERROR: PRIVMSG does not accept empty target !" << std::endl;
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
    std::cout << "Error: No such a client with the nickname:" << target << std::endl;
}

void Server::msgSendToChannel(int fd, std::string target, std::string message)
{
    (void)fd, (void)message;
    std::cout << "Broadcasting to channel: " << target << std::endl;
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
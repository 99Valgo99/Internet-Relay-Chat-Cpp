# include "Server.hpp"

void Server::validatePass(int fd, std::string arg)
{
    if (clients[fd].validPass)
    {
        sendServerReply(fd, 462, "Error: Client already registered !");
        return ;
    }
    if (arg == this->password)
        clients[fd].validPass = true;
    else
    {
        sendServerReply(fd, 464, "Error: Password Mismatch !");
        return ;
    }
}

void Server::validateUser(int fd, std::string username, std::string realname)
{
    if (!clients[fd].userName.empty())
    {
        sendServerReply(fd, 462, "Error: Client already issued an USER Command !");
        return ;
    }
    if (!clients[fd].validPass)
    {
        sendServerReply(fd, 451, "Error: Client has not registered yet !");
        return ;
    }
    if (username.empty())
    {
        sendServerReply(fd, 461, "Error: USER needs more parameters !");
        return ;
    }
    clients[fd].userName = username;
    clients[fd].realName = realname;
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
        sendServerReply(fd, 431, "Error: No Nickname Provided !");
        return ;
    }
    if (arg[0] == '#')
    {
        std::cerr << "Error: Can't start you nickname with '#', Only a channel do" << std::endl;
        return ;
    }
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->first == fd)
            continue ;
        if (it->second.nickName == arg)
        {
            sendServerReply(fd, 433, "Nickname is already taken ! chose something else");
            return ;
        }
    }
    clients[fd].nickName = arg;
}
# include "../Server/Server.hpp"

void Server::validatePass(int fd, std::string arg)
{
    if (clients[fd].validPass)
    {
        sendServerReply(fd, 462, "Error: Client already registered !");
        return ;
    }
    if (arg == this->password)
    {
        clients[fd].validPass = true;
        sendServerReply(fd, 0, "PASS: Password Confirmed !");
    }
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
    if (clients[fd].isClientAuth())
        sendWelcome(fd);
}

void Server::validateNick(int fd, std::string arg)
{
    if (!clients[fd].validPass)
    {
        sendServerReply(fd, 451, "Erro: Need a password before using NICK !");
        return ;
    }
    if (arg.empty())
    {
        sendServerReply(fd, 431, "Error: No Nickname Provided !");
        return ;
    }
    if (arg[0] == '#' || arg[0] == '&')
    {
        sendServerReply(fd, 999, "Error: Can't start your nickname with '#', only channels do !");
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
    std::string oldNickname = clients[fd].nickName;
    bool alreadyAuth = clients[fd].isClientAuth();
    clients[fd].nickName = arg;
    bool currAuth = clients[fd].isClientAuth();

    if (!alreadyAuth && currAuth)
        sendWelcome(fd);
    else if (alreadyAuth)
        sendChangeNick(fd, arg, oldNickname);
}
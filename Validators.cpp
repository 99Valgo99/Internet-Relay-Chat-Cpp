# include "Server.hpp"

void Server::validatePass(int fd, std::string arg)
{
    if (arg == this->password)
    {
        clients[fd].validPass = true;
        std::cout << "Passowrd Confirmed !" << std::endl;
    }
    else
    {
        std::cerr << "Error: Wrong Password" << std::endl;
        return ; // added
        // to develop more...
    }
}

void Server::validateUser(int fd, std::string username, std::string realname)
{
    if (!clients[fd].userName.empty())
    {
        std::cerr << "Error: Client Has Already Issued An USER Command !" << std::endl;
        return ;
    }
    if (!clients[fd].validPass)
    {
        std::cerr << "Error, Needs a password before using USER: PASS ****" << std::endl;
        return ;
    }
    if (username.empty())
    {
        std::cerr << "No Username Provided, Expected Format: USER username mode unused :realname" << std::endl;
        return ;
    }
    clients[fd].userName = username;
    clients[fd].realName = realname;
    std::cout << "Confirmed Username: " << clients[fd].userName << std::endl;
    std::cout << "Confirmed realname: " << clients[fd].realName << std::endl;
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
        std::cerr << "No Nickname Provided, Expected Format: NICK nickname" << std::endl;
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
            std::cerr << "Sorry ! Nickname Is Already Taken, Chose Something Else" << std::endl;
            return ;
        }
    }
    clients[fd].nickName = arg;
    std::cout << "Confirmed Nickname: " << clients[fd].nickName << std::endl;
}
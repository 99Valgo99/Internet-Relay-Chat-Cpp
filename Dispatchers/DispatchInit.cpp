# include "../Server/Server.hpp"

void Server::dispatchPass(int fd, std::istringstream& stream)
{
    std::string arg;
    std::getline(stream, arg);

    if (arg.empty())
    {
        sendServerReply(fd, 461, "Error: PASS should have one argument at least !");
        return ;
    }
    if (!arg.empty() && arg[0] == ' ')
        arg.erase(0, 1);
    if (!arg.empty() && arg[0] == ':')
    {
        arg.erase(0, 1);
        validatePass(fd, arg);
    }
    else
    {
        if (arg.find(' ') != std::string::npos)
        {
            sendServerReply(fd, 464, "Error: Password Mismatch !");
            return ;
        }
        validatePass(fd, arg);
    }
}

void Server::dispatchNick(int fd, std::istringstream& stream)
{
    std::string arg, leftovers;
    stream >> arg;
    if (stream >> leftovers)
        sendServerReply(fd, 461, "Error: NICK Should Have one argumet !");
    else
        validateNick(fd, arg);
}

void Server::dispatchUser(int fd, std::istringstream& stream)
{
    std::string username, mode, unused, realname;
    stream >> username >> mode >> unused;

    if (username.empty() || mode.empty() || unused.empty())
    {
        sendServerReply(fd, 461, "Error: User needs more parameters !");
        return ;
    }
    std::getline(stream, realname);
    if (!realname.empty() && realname[0] == ' ')
        realname.erase(0, 1);
    if (realname.empty() || realname[0] != ':')
    {
        sendServerReply(fd, 461, "Error: USER realname should start with ':' !");
        return ;
    }
    else
    {
        realname.erase(0, 1);
        validateUser(fd, username, realname);
    }
}
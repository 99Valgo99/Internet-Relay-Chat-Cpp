# include "../Server/Server.hpp"

void Server::dispatchPrvMsg(int fd, std::istringstream& stream)
{
    std::string targets, message;
    stream >> targets;
    if (targets.empty())
    {
        sendServerReply(fd, 461, "Error: PRIVMSG Needs more parameters !");
        return ;
    }
    std::getline(stream, message);
    if (!message.empty() && message[0] == ' ')
        message.erase(0, 1);
    if (message.empty() || message[0] != ':')
    {
        sendServerReply(fd, 412, "Error: PRIVMSG second arg format [:msg] !");
        return ;
    }
    handlePrvMsg(fd, targets, message);
}
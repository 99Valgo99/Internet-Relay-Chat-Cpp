# include "../Server/Server.hpp"

void Server::dispatchInvite(int fd, std::istringstream& stream)
{
    std::string nickname, channel, leftovers;
    stream >> nickname >> channel;
    if (nickname.empty() || channel.empty())
    {
        sendServerReply(fd, 461, "Error: INVITE Malformed input !");
        return ;
    }
    std::getline(stream, leftovers);
    if (!leftovers.empty() && leftovers[0] == ' ')
        leftovers.erase(0, 1);
    if (!leftovers.empty())
    {
        sendServerReply(fd, 999, "Error: INVITE requires only two args <nickname> and <channel> !");
        return ;
    }
    handleInvite(fd, nickname, channel);
}
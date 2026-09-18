# include "../Server/Server.hpp"

void Server::dispatchTopic(int fd, std::istringstream& stream)
{
    std::string channel, topic;
    stream >> channel;
    if (channel.empty())
    {
        sendServerReply(fd, 461, "Error: TOPIC must have at least one argument !");
        return ;
    }
    std::getline(stream, topic);
    if (!topic.empty() && topic[0] == ' ')
        topic.erase(0, 1);
    handleTopic(fd, channel, topic);
}
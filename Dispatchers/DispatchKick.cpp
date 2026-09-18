# include "../Server/Server.hpp"

void Server::dispatchKick(int fd, std::istringstream& stream)
{
    std::string channelList, usersList, comment;
    stream >> channelList >> usersList;
    if (channelList.empty() || usersList.empty())
    {
        sendServerReply(fd, 461, "Error: KICK needs a channel + users to kick !");
        return ;
    }
    std::getline(stream, comment);
    if (!comment.empty() && comment[0] == ' ')
        comment.erase(0, 1);
    if (!comment.empty())
    {
        if (comment[0] == ':')
            comment.erase(0, 1);
        else
        {
            sendServerReply(fd, 461, "Error: KICK Comment arg should start with ':' !");
            return ;
        }
    }
    size_t start = 0;
    std::vector<std::string> listChannel, listUsers;
    while (true)
    {
        std::string target;
        size_t posComma = channelList.find(',', start);
        if (posComma == std::string::npos)
        {
            target = channelList.substr(start);
            listChannel.push_back(target);
            break ;
        }
        target = channelList.substr(start, posComma - start);
        listChannel.push_back(target);
        start = posComma + 1;
    }
    start = 0;
    while (true)
    {
        std::string target;
        size_t posComma = usersList.find(',', start);
        if (posComma == std::string::npos)
        {
            target = usersList.substr(start);
            listUsers.push_back(target);
            break ;
        }
        target = usersList.substr(start, posComma - start);
        listUsers.push_back(target);
        start = posComma + 1;
    }
    handleKick(fd, listChannel, listUsers, comment);
}
# include "../Server/Server.hpp"

void Server::dispatchJoin(int fd, std::istringstream& stream)
{
    std::string channelname, keys, leftovers;
    stream >> channelname >> keys;
    if (channelname.empty())
    {
        sendServerReply(fd, 461, "Error: JOIN Channel must have a name !");
        return ;
    }
    if (channelname == "0")
    {
        exitAllChannels(fd);
        return ;
    }
    if (stream >> leftovers)
    {
        sendServerReply(fd, 999, "Error: JOIN should have one argument !");
        return ;
    }
    std::vector<std::string> keyholder;
    size_t start = 0;
    while (true)
    {
        std::string oneKey;
        size_t posComma = keys.find(',', start);
        if (posComma == std::string::npos)
        {
            oneKey = keys.substr(start);
            keyholder.push_back(oneKey);
            break ;
        }
        oneKey = keys.substr(start, posComma - start);
        keyholder.push_back(oneKey);
        start = posComma + 1;
    }
    start = 0;
    size_t key_index = 0;
    std::string emptyKey;
    while (true)
    {
        std::string getOneChannel;
        size_t posComma = channelname.find(',', start);
        if (posComma == std::string::npos)
        {
            getOneChannel = channelname.substr(start);
            if (key_index >= keyholder.size())
            {
                handleJoin(fd, getOneChannel, emptyKey);
                break ;
            }
            else
            {
                handleJoin(fd, getOneChannel, keyholder[key_index]);
                break ;
            }
        }
        getOneChannel = channelname.substr(start, posComma - start);
        if (key_index > keyholder.size())
            handleJoin(fd, getOneChannel, emptyKey);
        else
            handleJoin(fd, getOneChannel, keyholder[key_index]);
        start = posComma + 1;
        key_index++;
    }
}
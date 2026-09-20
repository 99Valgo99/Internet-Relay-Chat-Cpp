# include "../Server/Server.hpp"

void Server::dispatchMode(int fd, std::istringstream& stream)
{
    char sign;
    size_t threeArgCounter = 0;
    std::string modeStr, channelModed;
    stream >> channelModed >> modeStr;

    if (!channelModed.empty() && channelModed[0] != '#')
    {
        sendServerReply(fd, 461, "Error: MODE Needs more arguments !");
        return ;
    }
    
    std::string oneArgVector;
    std::vector<std::string> argsLeft;
    while (stream >> oneArgVector)
        argsLeft.push_back(oneArgVector);
    for (size_t i = 0; i < modeStr.size(); i++)
    {
        if (modeStr[i] == '-' || modeStr[i] == '+')
            sign = modeStr[i];
        else
        {
            if (modeStr[i] == 'i' || modeStr[i] == 'k' || modeStr[i] == 'l'
                || modeStr[i] == 'o' || modeStr[i] == 't')
            {
                bool needsArgBoolean = needAnArg(sign, modeStr[i]);
                if (needsArgBoolean)
                {
                    if (threeArgCounter + 1 > 3)
                    {
                        sendServerReply(fd, 999, "Error: MODE excess in use of flags that need args !");
                        return ;
                    }
                    bool argConsumption = getTheArg(argsLeft, threeArgCounter, oneArgVector);
                    if (!argConsumption)
                    {
                        sendServerReply(fd, 461, "Error: MODE Malformed input !");
                        return ;
                    }
                    handleMode(fd, channelModed, sign, modeStr[i], oneArgVector);
                }
                else
                {
                    std::string empty_arg;
                    handleMode(fd, channelModed, sign, modeStr[i], empty_arg);
                }
            }
            else
                sendServerReply(fd, 472, "Error: Mode Unknown Mode !");
        }
    }
}
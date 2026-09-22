# include "../Server/Server.hpp"

std::string Server::buildSenderPrifix(int fd)
{
    std::string Sender = ":" + this->clients[fd].nickName + "!" + this->clients[fd].userName + "@localhost";
    return Sender;
}

void Server::msgSendToNick(int fd, std::string target, std::string message)
{
    if (target.empty())
    {
        sendServerReply(fd, 461, "ERROR: PRIVMSG needs more parameters !");
        return ;
    }
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second.nickName == target)
        {
            std::cout << "Sending to Client: (updated output) -> " << it->second.nickName << std::endl;
            std::string sender = buildSenderPrifix(fd);
            it->second.sendBytes.append(sender + " PRIVMSG " + target + " :" + message + "\r\n");
            return ;
        }
    }
    sendServerReply(fd, 401, "Error: No such a client with the nickname:" + target);
}

void Server::msgSendToChannel(int fd, std::string target, std::string message)
{
    std::cout << "Broadcasting to channel: " << target << std::endl;
    if (target.empty())
    {
        sendServerReply(fd, 461, "ERROR: PRIVMSG needs more parameters !");
        return ;
    }
    std::string originalName = target;
    lowerChannelName(target);
    std::map<std::string, Channel>::iterator it = this->channels.find(target);
    if (it == this->channels.end())
    {
        sendServerReply(fd, 404, "Error: No Channel was found with this name !");
        return ;
    }
    else
    {
        std::string sender = buildSenderPrifix(fd);
        const std::set<int>& members = it->second.getChannelsMembers();
        if (members.find(fd) == members.end())
        {
            sendServerReply(fd, 404, "Error: The client has not joined this channel !");
            return ;
        }
        for (std::set<int>::const_iterator it_members = members.begin(); it_members != members.end(); ++it_members)
        {
            if (*it_members == fd)
                continue ;
            std::map<int, Client>::iterator guard = this->clients.find(*it_members);
            if (guard == clients.end())
                continue ;
            guard->second.sendBytes.append(sender + " PRIVMSG " + originalName + " :" + message + "\r\n");
        }
    }
}

bool Server::spreadMessage(int fd)
{
    int sendResult = send(fd, clients[fd].sendBytes.c_str(), clients[fd].sendBytes.size(), 0);
    if (sendResult == -1)
    {
        close(fd);
        return false;
    }
    clients[fd].sendBytes.erase(0, sendResult);
    return true;
}

void Server::sendServerReply(int fd, int code, std::string message)
{
    std::map<int, Client>::iterator it_client = this->clients.find(fd);
    if (it_client == this->clients.end())
        return ;
    std::ostringstream reply;
    reply << ":ircserv " << std::setfill('0') << std::setw(3) << code << " " << it_client->second.nickName << " :" << message;
    it_client->second.sendBytes.append(reply.str() + "\r\n");
}

void Server::sendServerReplyarg(int fd, int code, std::string arg, std::string message)
{
    std::map<int, Client>::iterator it_client = this->clients.find(fd);
    if (it_client == this->clients.end())
        return ;
    std::ostringstream reply;
    reply << ":ircserv " << code << " " << it_client->second.nickName << " " << arg << " :" << message;
    it_client->second.sendBytes.append(reply.str() + "\r\n");
}

void Server::sendWelcome(int fd)
{
    std::string welMsg = "Welcome to the internet relay network chat " + clients[fd].nickName + "!" + clients[fd].userName + "@localhost";
    sendServerReply(fd, 1, welMsg);
}

void Server::sendChangeNick(int fd, std::string arg, std::string oldNickname)
{
    std::string changeMsg = oldNickname + "!" + clients[fd].userName + "@localhost " + "NICK " + arg;
    sendServerReply(fd, 0, changeMsg);
}

void Server::sendChanneljoin(int fd, Channel& channel)
{
    std::string joinMsg = "Welcome to " + channel.getChannelOriginalName();
    sendServerReply(fd, 332, joinMsg);
    if (!channel.getTopic().empty())
    {
        std::string topicmsg = "The channel's Topic is: " + channel.getTopic();
        sendServerReply(fd, 332, topicmsg);
    }
}

void Server::invitationMsg(int sender, int invited, std::string channelname)
{
    std::string invMsg = clients[sender].nickName + "!" + clients[sender].userName + "@localhost" + " INVITE " + clients[invited].nickName + " " + channelname;
    sendServerReply(invited, 0, invMsg);
}

void Server::broadcastJoin(int joined, Channel& channel)
{
    std::string joinMsg = clients[joined].nickName + "!" + clients[joined].userName + "@localhost" + " JOIN " + channel.getChannelOriginalName();
    
    for (std::set<int>::iterator members = channel.getChannelsMembers().begin(); members != channel.getChannelsMembers().end(); ++members)
    {
        if (*members == joined)
            continue;
        sendServerReply(*members, 0, joinMsg);
    }
}

void Server::broadcastExit(int clientLeft, Channel& channel)
{
    for (std::set<int>::iterator members = channel.getChannelsMembers().begin(); members != channel.getChannelsMembers().end(); ++members) // Hmmmm, no Const?
    {
        if (*members == clientLeft)
            continue ;
        std::string exitMsg = clients[clientLeft].nickName + "!" + clients[clientLeft].userName + "@localhost" + " JOIN 0 " + channel.getChannelOriginalName();
        sendServerReply(*members, 0, exitMsg);
    }
}

void Server::broadcastKick(int kicker, int kicked, Channel& channel, std::string comment)
{
    for (std::set<int>::iterator member = channel.getChannelsMembers().begin(); member != channel.getChannelsMembers().end(); ++member)
    {
        std::string kickMsg = clients[kicker].nickName + "!" + clients[kicker].userName + "@localhost" + " KICK " + channel.getChannelOriginalName() + " " + clients[kicked].nickName;
        if (!comment.empty())
            kickMsg.append(" :" + comment);
        sendServerReply(*member, 0, kickMsg);
        // sendServerReply(kicked, 0, kickMsg); // why does the kick message arive twice, and when this line is removed, it does not arrive at all?
    }
}

void Server::broadcastTopic(int fd, Channel& channel)
{
    std::string topicmsg = clients[fd].nickName + "!" + clients[fd].userName + "@localhost" + " TOPIC " + channel.getChannelOriginalName() + " :" + channel.getTopic();
    sendServerReply(fd, 0, topicmsg);
}

void Server::sendModeMsg(int fd, char flag, Channel& channel, bool toggle)
{
    std::string modeMsg = clients[fd].nickName + "!" + clients[fd].userName + "@localhost" + " MODE " + channel.getChannelOriginalName() + " " + flag;
    if (toggle)
        modeMsg.append(": Has been activated !");
    else
        modeMsg.append(": Has been deactivated !");
    sendServerReply(fd, 0, modeMsg);
}
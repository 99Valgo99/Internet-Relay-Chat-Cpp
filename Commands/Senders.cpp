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
    sendServerReply(fd, 401, "Error: No such a client with the nickname:");
}

void Server::msgSendToChannel(int fd, std::string target, std::string message)
{
    std::cout << "Broadcasting to channel: " << target << std::endl;
    if (target.empty())
    {
        sendServerReply(fd, 461, "ERROR: PRIVMSG needs more parameters !");
        return ;
    }
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
            this->clients[*it_members].sendBytes.append(sender + " PRIVMSG " + target + " :" + message + "\r\n");
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
    std::ostringstream reply;
    reply << "ircserv " << code << " " << it_client->second.nickName << " :" << message;
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
    std::string joinMsg = "Welcome to " + channel.getChannelsName();
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
    std::string joinMsg = clients[joined].nickName + "!" + clients[joined].userName + "@localhost" + " JOIN " + channel.getChannelsName();
    
    for (std::set<int>::iterator members = channel.getChannelsMembers().begin(); members != channel.getChannelsMembers().end(); ++members)
    {
        if (*members == joined)
            continue;
        sendServerReply(*members, 0, joinMsg);
    }
}
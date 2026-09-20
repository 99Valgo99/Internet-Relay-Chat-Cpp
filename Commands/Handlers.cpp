# include "../Server/Server.hpp"

void Server::exitAllChannels(int fd)
{
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        std::set<int>::const_iterator member = it->second.getChannelsMembers().find(fd);
        if (member == it->second.getChannelsMembers().end())
            continue;
        else
        {
            it->second.removeClientsFromChannel(fd);
            it->second.removeOperator(fd);
            broadcastExit(fd, it->second);
        }
    }
}

void Server::handleJoin(int fd, std::string nameChannel, std::string key)
{
    if (!clients[fd].isClientAuth())
    {
        sendServerReply(fd, 451, "Error: Client is not authenicated yet !");
        return ;
    }
    if (nameChannel.empty() || (nameChannel[0] != '#' && nameChannel[0] != '&'))
    {
        sendServerReply(fd, 403, "Error: JOIN No such a channel !");
        return ;
    }
    std::map<std::string, Channel>::iterator it = channels.find(nameChannel);
    if (it == channels.end())
    {
        if (!key.empty())
        {
            sendServerReply(fd, 999, "Error: Malformed JOIN argument, to set a key for a channel, use MODE #example +k");
            return ;
        }
        Channel newChannel(nameChannel, fd);
        std::set<int> ops = newChannel.getOperators();
        this->channels.insert(std::make_pair(nameChannel, newChannel));
        sendChanneljoin(fd, newChannel);
        return ;
    }
    if (it->second.getInviteToggle())
    {
        std::set<int>::const_iterator invitedMemb = it->second.getInvitedMembers().find(fd);
        if (invitedMemb == it->second.getInvitedMembers().end())
        {
            sendServerReply(fd, 473, "Error: The channel is Invite-Only, and the sender was not invited !");
            return ;
        }
    }
    if (key == it->second.getChannelPassword() || it->second.getChannelPassword().empty())
    {
        if (it->second.getUserLimit() != -1 && it->second.getChannelsMembers().size() >= (size_t)it->second.getUserLimit())
        {
            sendServerReply(fd, 471, "Error: Joining would exceed the user limit number of this channel !");
            return ; 
        }
        it->second.addClientsToChannel(fd);
        sendChanneljoin(fd, it->second);
        broadcastJoin(fd, it->second);
    }
    else
        sendServerReply(fd, 475, "This Channel needs a password key to join it !");
}

void Server::handlePrvMsg(int fd, std::string targets, std::string message)
{
    if (!clients[fd].isClientAuth())
    {
        sendServerReply(fd, 451, "Error: Client is not authenicated yet !");
        return ;
    }
    
    message.erase(0, 1);
    size_t start = 0;
    bool lastOne = false;
    while (!lastOne)
    {
        std::string oneTarget;
        size_t posComma = targets.find(',', start);
        if (posComma == std::string::npos)
        {
            oneTarget = targets.substr(start);
            lastOne = true;
        }
        else
        {
            oneTarget = targets.substr(start, posComma - start);
            start = posComma + 1;
        }
        if (!oneTarget.empty() && oneTarget[0] == '#')
            msgSendToChannel(fd, oneTarget, message);
        else
            msgSendToNick(fd, oneTarget, message);
    }
}


void Server::kickOneClient(int fd, std::string listChannel, std::string listUser)
{
    if (!listChannel.empty())
    {
        sendServerReply(fd, 403, "Error: KICK No such a channel !");
        return ;
    }
    if (listChannel[0] != '#' && listChannel[0] != '&')
    {
        sendServerReply(fd, -1, "Error: KICK malformed channel's name !");
        return ;
    }
    if (listChannel.empty() || listUser.empty())
    {
        sendServerReply(fd, 461, "Error: KICK needs more parameters !");
        return ;
    }
    std::map<std::string, Channel>::iterator channel_it = channels.find(listChannel);
    if (channel_it == channels.end())
    {
        sendServerReply(fd, 403, "Error: KICK No such a channel !");
        return ;
    }
    std::set<int>::const_iterator isOp = channel_it->second.getOperators().find(fd);
    if (isOp == channel_it->second.getOperators().end())
    {
        sendServerReply(fd, 482, "Error: KICK The kicker is not an operator !");
        return ;
    }
    int user_fd = -1;
    for (std::map<int, Client>::iterator user_it = clients.begin(); user_it != clients.end(); ++user_it)
    {
        if (user_it->second.nickName == listUser)
            user_fd = user_it->first;
    }
    if (user_fd == -1)
    {
        sendServerReply(fd, 401, "Error: KICK Nickname does not exist !");
        return ;
    }
    std::set<int>::const_iterator member = channel_it->second.getChannelsMembers().find(user_fd);
    if (member == channel_it->second.getChannelsMembers().end())
    {
        sendServerReply(fd, 441, "Error: KICK User not in channel !");
        return ;
    }
    else
        channel_it->second.removeClientsFromChannel(*member);
    std::set<int>::const_iterator opMember = channel_it->second.getOperators().find(user_fd);
    if (opMember != channel_it->second.getOperators().end())
        channel_it->second.removeOperator(*opMember);
    std::cout << listUser << " Was Kicked of Channel: " << listChannel << std::endl;
}

void Server::handleKick(int fd, std::vector<std::string> listChannel, std::vector<std::string> listUsers, std::string comment)
{
    (void)comment;
    if (!clients[fd].isClientAuth())
    {
        sendServerReply(fd, 451, "Error: Client is not authenicated yet !");
        return ;
    }
    if (!(listChannel.size() == 1 || listChannel.size() == listUsers.size()))
    {
        sendServerReply(fd, 461, "Error: KICK Needs more parameters !");
        return ;
    }
    if (listChannel.size() == 1)
    {
        for (size_t index = 0; index < listUsers.size(); index++)
            kickOneClient(fd, listChannel[0], listUsers[index]);
    }
    else
    {
        for (size_t index = 0; index < listChannel.size(); index++)
            kickOneClient(fd, listChannel[index], listUsers[index]);
    }
}

void Server::handleTopic(int fd, std::string channelT, std::string _topic)
{
    if (!clients[fd].isClientAuth())
    {
        sendServerReply(fd, 451, "Error: Client is not authenicated yet !");
        return ;
    }
    std::map<std::string, Channel>::iterator it = this->channels.find(channelT);
    if (it == channels.end())
    {
        sendServerReply(fd, 403, "Error: TOPIC No such a channel !");
        return ;
    }
    std::set<int>::const_iterator member = it->second.getChannelsMembers().find(fd);
    if (member == it->second.getChannelsMembers().end())
    {
        sendServerReply(fd, 442, "Error: TOPIC This Client is not a member in this Channel !");
        return ;
    }
    if (_topic.empty())
    {
        std::string topicmsg = "The channel's Topic is: " + it->second.getTopic();
        sendServerReply(fd, 332, topicmsg);
        return ;
    }
    else if (_topic[0] == ':')
    {
        if (it->second.getTopicToggle())
        {
            std::set<int>::const_iterator opMember = it->second.getOperators().find(fd);
            if (opMember == it->second.getOperators().end())
            {
                sendServerReply(fd, 482, "Error: TOPIC Client must be an operator !");
                return ;
            }
        }
        _topic.erase(0, 1);
        if (_topic.empty())
            it->second.setTopic("");
        else
            it->second.setTopic(_topic);
    }
    else
    {
        sendServerReply(fd, -1, "Error: TOPIC Malformed input !");
        return ;
    }
}

void Server::handleInvite(int fd, std::string _nickname, std::string _channel)
{
    if (!clients[fd].isClientAuth())
    {
        sendServerReply(fd, 451, "Error: Client is not authenicated yet !");
        return ;
    }
    if (_nickname[0] == '#' || _nickname[0] == '&')
    {
        sendServerReply(fd, 401, "Error: INVITE Invalid Nickname !");
        return ;
    }
    if (_channel[0] != '#' && _channel[0] != '&')
    {
        sendServerReply(fd, 403, "Error: INVITE No such a channel !");
        return ;
    }
    int user_fd = -1;
    for (std::map<int, Client>::iterator cl_it = clients.begin(); cl_it != clients.end(); ++cl_it)
    {
        if (cl_it->second.nickName == _nickname)
            user_fd = cl_it->first;
    }
    if (user_fd == -1)
    {
        sendServerReply(fd, 401, "Error: INVITE No Such a nickname !");
        return ;
    }
    std::map<std::string, Channel>::iterator it = channels.find(_channel);
    if (it == channels.end())
    {
        sendServerReply(fd, 341, "INVITE: Sending the invitation... !");
        invitationMsg(fd, user_fd, _channel);
        return ;
    }
    else
    {
        std::set<int>::const_iterator inviter_member = it->second.getChannelsMembers().find(fd);
        if (inviter_member == it->second.getChannelsMembers().end())
        {
            sendServerReply(fd, 442, "Error: INVITE Inviter is not a member of this channel !");
            return ;
        }
        std::set<int>::const_iterator member = it->second.getChannelsMembers().find(user_fd);
        if (member == it->second.getChannelsMembers().end())
        {
            it->second.addInvited(user_fd);
            sendServerReply(fd, 341, "INVITE: Sending the invitation... !");
            invitationMsg(fd, user_fd, it->second.getChannelsName());
            return ;
        }
        else
        {
            sendServerReply(fd, 443, "Error: INVITE the invited client is already a member of this channel !");
            return ;
        }
    }
}

bool Server::userLimitHelper(Channel& _Channel, std::string arg)
{
    for (size_t i = 0; i < arg.size(); i++)
    {
        if (!std::isdigit(arg[i]))
            return false;
    }
    char *endptr;
    long value = std::strtol(arg.c_str(), &endptr, 10);
    if (*endptr != '\0'|| value < 0 || value == LONG_MAX)
        return false;
    _Channel.setUserLimit(value);
    return true;
}

void Server::operatorModeHelper(int fd, Channel& _Channel, bool& toggle, std::string& arg)
{
    int user_fd = -1;
    for (std::map<int, Client>::iterator client_it = clients.begin(); client_it != clients.end(); ++client_it)
    {
        if (client_it->second.nickName == arg)
        {
            user_fd = client_it->first;
            break;
        }
    }
    if (user_fd == -1)
    {
        sendServerReply(fd, 401, "Error: MODE NO such a nickname !");
        return ;
    }
    std::set<int>::const_iterator member_it = _Channel.getChannelsMembers().find(user_fd);
    if (member_it == _Channel.getChannelsMembers().end())
    {
        sendServerReply(fd, 442, "Error: MODE client is not a member of this channel !");
        return ;
    }
    if (toggle)
        _Channel.addOperators(*member_it);
    else
        _Channel.removeOperator(*member_it);
}

void Server::handleMode(int fd, std::string channel_, char sign, char flag, std::string arg)
{
    std::map<std::string, Channel>::iterator channel_it = channels.find(channel_);
    if (channel_it == channels.end())
    {
        sendServerReply(fd, 403, "Error: MODE No such a channel !");
        return ;
    }
    std::set<int>::const_iterator send_it = channel_it->second.getOperators().find(fd);
    if (send_it == channel_it->second.getOperators().end())
    {
        sendServerReply(fd, 482, "Error: MODE Client must be an operator !");
        return ;
    }
    bool toggle = false;
    if (sign == '+') toggle = true;
    else toggle = false;
    if (arg.empty())
    {
        if (flag == 'i')
            channel_it->second.toggleInvite(toggle);
        else if (flag == 't')
            channel_it->second.toggleTopic(toggle);
        if (flag == 'l' && !toggle)
            channel_it->second.setUserLimit(-1);
    }
    else if (!arg.empty())
    {
        if (flag == 'l' && toggle)
        {
            bool invalid = userLimitHelper(channel_it->second, arg);
            if (!invalid)
            {
                sendServerReply(fd, 461, "Error: MODE invalid argument for l flag !");
                return ;
            }
        }
        else if (flag == 'k')
        {
            if (!toggle)
                channel_it->second.setChannelPassword("");
            else
                channel_it->second.setChannelPassword(arg);
        }
        else if (flag == 'o')
            operatorModeHelper(fd, channel_it->second, toggle, arg);
    }
}
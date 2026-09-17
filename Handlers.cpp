# include "Server.hpp"

void Server::exitAllChannels(int fd)
{
    bool notMemberOfAny = true;
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        std::set<int>::const_iterator member = it->second.getChannelsMembers().find(fd);
        if (member == it->second.getChannelsMembers().end())
            continue;
        else
        {
            it->second.removeClientsFromChannel(fd);
            it->second.removeOperator(fd);
            notMemberOfAny = false;
            std::cout << "A client has left " << it->second.getChannelsName() << std::endl;
        }
    }
    if (notMemberOfAny == true)
        std::cout << "The client is not a memebre of any channel !" << std::endl;
}

void Server::handleJoin(int fd, std::string nameChannel, std::string key)
{
    if (!clients[fd].isClientAuth())
    {
        std::cerr << "Error: You are not authenticated yet !" << std::endl;
        return ;
    }
    if (nameChannel.empty() || nameChannel[0] != '#')
    {
        std::cerr << "Error: A channel should always start with '#' !" << std::endl;
        return ;
    }
    std::cout << "Channel -> " << nameChannel << " | Key -> " << key << std::endl;
    std::map<std::string, Channel>::iterator it = channels.find(nameChannel);
    if (it == channels.end())
    {
        if (!key.empty())
        {
            std::cerr << "Error: Malformed JOIN argument, to set a key for a channel, use MODE #example +k" << std::endl;
            return ;
        }
        Channel newChannel(nameChannel, fd);
        std::set<int> ops = newChannel.getOperators();
        std::cout << "Channel's first Operator/Creator: " << this->clients[*ops.begin()].nickName << std::endl;
        this->channels.insert(std::make_pair(nameChannel, newChannel));
        std::cout << "Channel was created and client was added to channel: " << nameChannel << std::endl;
        return ;
    }
    if (it->second.getInviteToggle())
    {
        std::set<int>::const_iterator invitedMemb = it->second.getInvitedMembers().find(fd);
        if (invitedMemb == it->second.getInvitedMembers().end())
        {
            std::cerr << "Error: The channel is Invite-Only, and the sender was not invited !" << std::endl;
            return ;
        }
    }
    if (key == it->second.getChannelPassword() || it->second.getChannelPassword().empty())
    {
        it->second.addClientsToChannel(fd);
        std::cout << "Client was added to the existing channel: " << nameChannel << std::endl;
    }
    else
        std::cerr << "This Channel needs a password key to join it !" << std::endl;
}

void Server::handlePrvMsg(int fd, std::string targets, std::string message)
{
    std::cout << "PRVMSG was succesfully called !" << std::endl;
    if (!clients[fd].isClientAuth())
    {
        std::cerr << "Error: You are not authenticated yet !" << std::endl;
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
    if (!listChannel.empty() && listChannel[0] != '#')
    {
        std::cerr << "Error: Malformed Channel syntax !" << std::endl;
        return ;
    }
    if (listChannel.empty() || listUser.empty())
    {
        std::cerr << "Error: Malformed Kick input" << std::endl;
        return ;
    }
    std::map<std::string, Channel>::iterator channel_it = channels.find(listChannel);
    if (channel_it == channels.end())
    {
        std::cerr << "Error: " << listChannel << " was not created !" << std::endl;
        return ;
    }
    std::set<int>::const_iterator isOp = channel_it->second.getOperators().find(fd);
    if (isOp == channel_it->second.getOperators().end())
    {
        std::cerr << "Error: The kicker is not an operator of the " << listChannel << " channel !" << std::endl;
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
        std::cerr << "Error: Client does not exist !" << std::endl;
        return ;
    }
    std::set<int>::const_iterator member = channel_it->second.getChannelsMembers().find(user_fd);
    if (member == channel_it->second.getChannelsMembers().end())
    {
        std::cerr << "Error: " << listUser << " is not part of the channel " << listChannel << "!" << std::endl;
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
        std::cerr << "Error: You are not authenticated yet !" << std::endl;
        return ;
    }
    if (!(listChannel.size() == 1 || listChannel.size() == listUsers.size()))
    {
        std::cerr << "Error: Malformed input for Kick !" << std::endl;
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
        std::cerr << "Error: You are not authenticated yet !" << std::endl;
        return ;
    }
    std::map<std::string, Channel>::iterator it = this->channels.find(channelT);
    if (it == channels.end())
    {
        std::cerr << "Error: Channel speicifed in TOPIC does not exist !" << std::endl;
        return ;
    }
    std::set<int>::const_iterator member = it->second.getChannelsMembers().find(fd);
    if (member == it->second.getChannelsMembers().end())
    {
        std::cerr << "Error: This Client is not a member in this Channel" << std::endl;
        return ;
    }
    if (_topic.empty())
    {
        std::cout << "Channel's topic is: " << it->second.getTopic() << std::endl;
        return ;
    }
    else if (_topic[0] == ':')
    {
        _topic.erase(0, 1);
        if (_topic.empty())
            it->second.setTopic("");
        else
            it->second.setTopic(_topic);
    }
    else
    {
        std::cerr << "Error: Malformed input for TOPIC" << std::endl;
        return ;
    }
}

void Server::handleInvite(int fd, std::string _nickname, std::string _channel)
{
    if (!clients[fd].isClientAuth())
    {
        std::cerr << "Error: You are not authenticated yet !" << std::endl;
        return ;
    }
    if (_nickname[0] == '#')
    {
        std::cerr << "Error: Invalid Nickname !" << std::endl;
        return ;
    }
    if (_channel[0] != '#')
    {
        std::cerr << "Error: Invalid Channel's name !" << std::endl;
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
        std::cerr << "Error: The client with this nickname does not exist !" << std::endl;
        return ;
    }
    std::map<std::string, Channel>::iterator it = channels.find(_channel);
    if (it == channels.end())
    {
        std::cout << "Sending the invitation to: " << _nickname << std::endl;
        // broadcast it here later...
        return ;
    }
    else
    {
        std::set<int>::const_iterator inviter_member = it->second.getChannelsMembers().find(fd);
        if (inviter_member == it->second.getChannelsMembers().end())
        {
            std::cerr << "Error: The inviter client is not a memeber of this channel !" << std::endl;
            return ;
        }
        std::set<int>::const_iterator member = it->second.getChannelsMembers().find(user_fd);
        if (member == it->second.getChannelsMembers().end())
        {
            std::cout << "Inviting " << _nickname << std::endl;
            it->second.addInvited(user_fd);
            // broadcast here later..
            return ;
        }
        else
        {
            std::cout << "Error: the invited client is already a member of this channel !" << std::endl;
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

void Server::operatorModeHelper(Channel& _Channel, bool& toggle, std::string& arg)
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
        std::cerr << "Error: MODE Client does not exist !" << std::endl;
        return ;
    }
    std::set<int>::const_iterator member_it = _Channel.getChannelsMembers().find(user_fd);
    if (member_it == _Channel.getChannelsMembers().end())
    {
        std::cerr << "Error: Mode the provided Client is not a member of this channel !" << std::endl;
        return ;
    }
    if (toggle)
        _Channel.addOperators(*member_it);
    else
        _Channel.removeOperator(*member_it);
}

void Server::handleMode(int fd, std::string channel_, char sign, char flag, std::string arg)
{
    // checking if the sender is part of the channel
    std::map<std::string, Channel>::iterator channel_it = channels.find(channel_);
    if (channel_it == channels.end())
    {
        std::cerr << "ERROR: MODE channel does not exist !" << std::endl;
        return ;
    }
    std::set<int>::const_iterator send_it = channel_it->second.getOperators().find(fd);
    if (send_it == channel_it->second.getOperators().end())
    {
        std::cerr << "ERROR: MODE The sender is not an operator in this channel !" << std::endl;
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
                std::cerr << "Error: MODE invalid argument for l flag" << std::endl;
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
            operatorModeHelper(channel_it->second, toggle, arg);
    }
}
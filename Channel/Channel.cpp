# include "Channel.hpp"

Channel::Channel(std::string name, int fd, std::string _originalName) {
    this->userLimit = -1;
    this->channelsName = name;
    this->fds_list.insert(fd);
    this->operators.insert(fd);
    this->inviteOnly = false;
    this->topicToggle = false;
    this->originalName = _originalName;
}

void Channel::addClientsToChannel(int fd) {
    this->fds_list.insert(fd);
}

void Channel::removeClientsFromChannel(int fd) {
    this->fds_list.erase(fd);
}

void Channel::setTopic(std::string addedTopic)
{
    this->topic = addedTopic;
}

const std::string& Channel::getChannelsName() const {
    return (this->channelsName);
}

const std::set<int>& Channel::getChannelsMembers() const {
    return (this->fds_list);
}

const std::set<int>& Channel::getOperators() const {
    return (this->operators);
}

void Channel::addOperators(int fd) {
    this->operators.insert(fd);
}

void Channel::addInvited(int fd) {
    this->invited.insert(fd);
}

const std::set<int>& Channel::getInvitedMembers() const {
    return (this->invited);
}

const std::string& Channel::getTopic() const {
    return (this->topic);
}

void Channel::removeOperator(int fd)
{
    this->operators.erase(fd);
    if (operators.empty())
    {
        if (!fds_list.empty())
            this->operators.insert(*fds_list.begin());
    }
}

void Channel::toggleInvite(bool toggle) {
    this->inviteOnly = toggle;
}

void Channel::toggleTopic(bool toggle) {
    this->topicToggle = toggle;
}

const bool& Channel::getInviteToggle() const {
    return (this->inviteOnly);
}

const bool& Channel::getTopicToggle() const {
    return (this->topicToggle);
}

void Channel::setUserLimit(long limit) {
    this->userLimit = limit;
}

const long& Channel::getUserLimit() const {
    return (this->userLimit);
}

void Channel::setChannelPassword(std::string _password) {
    this->password = _password;
}

const std::string& Channel::getChannelPassword() const {
    return (this->password);
}

const std::string& Channel::getChannelOriginalName() const {
    return (this->originalName);
}
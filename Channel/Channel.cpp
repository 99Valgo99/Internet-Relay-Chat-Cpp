# include "Channel.hpp"

Channel::Channel(std::string name, int fd) {
    this->userLimit = -1;
    this->channelsName = name;
    this->fds_list.insert(fd);
    this->operators.insert(fd);
    this->inviteOnly = false;
    this->topicToggle = false;
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
    std::cout << "Client: " << fd << " was made an operator on this channel !" << std::endl;
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
    std::cout << "Client: " << fd << " is no longer an operator on this channel !" << std::endl;
    this->operators.erase(fd);
}

void Channel::toggleInvite(bool toggle) {
    this->inviteOnly = toggle;
    std::cout << this->channelsName << "Invite toggled to: " << toggle << std::endl;
}

void Channel::toggleTopic(bool toggle) {
    this->topicToggle = toggle;
    std::cout << this->channelsName << "Topic toggled to: " << toggle << std::endl;
}

const bool& Channel::getInviteToggle() const {
    return (this->inviteOnly);
}

const bool& Channel::getTopicToggle() const {
    return (this->topicToggle);
}

void Channel::setUserLimit(long limit) {
    std::cout << "Setting user limit at: " << limit << " for Channel: " << this->channelsName << std::endl;
    this->userLimit = limit;
}

const long& Channel::getUserLimit() const {
    return (this->userLimit);
}

void Channel::setChannelPassword(std::string _password) {
    std::cout << "Password: " << _password << " is set for channel: " << this->channelsName << std::endl;
    this->password = _password;
}

const std::string& Channel::getChannelPassword() const {
    return (this->password);
}
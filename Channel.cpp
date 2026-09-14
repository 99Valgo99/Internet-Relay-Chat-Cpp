# include "Channel.hpp"

Channel::Channel(std::string name, int fd) {
    this->channelsName = name;
    this->fds_list.insert(fd);
    this->operators.insert(fd);
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
}
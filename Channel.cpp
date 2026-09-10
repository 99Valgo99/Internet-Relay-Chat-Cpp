# include "Channel.hpp"

Channel::Channel(std::string name, int fd) {
    this->channelsName = name;
    this->fds_list.insert(fd);
}

void Channel::addClientsToChannel(int fd) {
    this->fds_list.insert(fd);
}

void Channel::removeClientsFromChannel(int fd) {
    this->fds_list.erase(fd);
}

const std::string& Channel::getChannelsName() const {
    return (this->channelsName);
}

const std::set<int>& Channel::getChannelsMembers() const {
    return (this->fds_list);
}


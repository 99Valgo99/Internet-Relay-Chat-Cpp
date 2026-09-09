# include "Channel.hpp"

Channel::Channel(std::string name, int fd) {
    this->channelsName = name;
    this->fds_list.insert(fd);
}

void addClientsToChannel(int fd) {
    this->fds_list.insert(fd);
}

void removeClientsFromChannel(int fd) {
    thos->fds_list.erase(fd);
}

const std::string& Channel::getChannelsName() const {
    return (this->channelsName);
}

const std::set<int>& getChannelsMembers() const {
    return (this->fds_list);
}


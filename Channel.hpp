# ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <set>
# include <string>

class Channel {

    private:
        std::set<int> fds_list;
        std::string channelsName;
    public:
        void addClientsToChannel(int fd);
        void removeClientsFromChannel(int fd);
        const std::string& getChannelsName() const;
        const std::set<int>& getChannelsMembers() const;
        Channel(std::string name, int fd);
};


# endif
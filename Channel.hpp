# ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <set>
# include <string>

class Channel {

    private:
        std::string topic;
        std::string channelsName;
        std::set<int> fds_list;
        std::set<int> operators;
        std::set<int> invited;
    
    public:
        Channel(std::string name, int fd);
        
        void addClientsToChannel(int fd);
        void removeClientsFromChannel(int fd);

        void setTopic(std::string addedTopic);
        const std::string& getTopic() const;
        const std::string& getChannelsName() const;
        const std::set<int>& getChannelsMembers() const;
        
        void addOperators(int fd);
        const std::set<int>& getOperators() const;

        void addInvited(int fd);
        const std::set<int>& getInvitedMembers() const;

        void removeOperator(int fd);

};


# endif
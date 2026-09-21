# ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <set>
# include <string>
# include <iostream>

class Channel {

    private:
        bool inviteOnly;
        bool topicToggle;
        long userLimit;
        std::string password;
        std::string topic;
        std::string channelsName;
        std::string originalName;
        std::set<int> fds_list;
        std::set<int> operators;
        std::set<int> invited;
    
    public:
        Channel(std::string name, int fd, std::string _originalName);
        
        void addClientsToChannel(int fd);
        void removeClientsFromChannel(int fd);

        void setTopic(std::string addedTopic);
        const std::string& getTopic() const;
        const std::string& getChannelsName() const;
        const std::set<int>& getChannelsMembers() const;
        
        void addOperators(int fd);
        void removeOperator(int fd);
        const std::set<int>& getOperators() const;

        void addInvited(int fd);
        const std::set<int>& getInvitedMembers() const;

        void toggleInvite(bool toggle);
        const bool& getInviteToggle() const;

        void toggleTopic(bool toggle);
        const bool& getTopicToggle() const;

        void setUserLimit(long limit);
        const long& getUserLimit() const;

        void setChannelPassword(std::string _password);
        const std::string& getChannelPassword() const;

        const std::string& getChannelOriginalName() const;
};


# endif
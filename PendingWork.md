## Pending work

### Done

* Registration welcome sequence (001-004) — design just settled, not yet written (Done)
* Nick-change confirmation — same, design settled, not written (Done)
* JOIN success sequence (332/331 topic, 353/366 member list) (Done)
* INVITE success (341 RPL_INVITING) (Done)
* Universal channel-broadcast function (JOIN confirmation to channel (Done), KICK notification (Done), JOIN 0/PART notification (Done))
* KICK's comment argument — parsed but unused, needs wiring into its eventual broadcast (Done)
* TOPIC message to the sender of the command (Done)
* Need to check for channel re JOINING (Done)
* flushing Partial commands using nc -C + CTRL + D test from subject (Done).
* in case the operator has left the channel and no op left, the first client joined after the operator and not an op becomes an op. (Done)
* Signals handling && Clean fds once the server gets CTRL + C or other ending signals. (Done)
* Hostname placeholder decision ("localhost" vs real IP) (Done)
* Nickname character-set/length grammar validation. (Done)
* Channel name case-insensitivity fix (Done)
* PRIVMSG embedded \r\n injection check (Done)

### Working On:

* Real reference client testing (WeeChat/irssi/HexChat) — haven't touched this at all yet, and it's arguably the most important remaining item since it validates everything else

### Before push

* Inspect channel's lowercasing accross access controls. (Done)
* removal of cout debuggers. (Done)
* removal of all comments.

### To test now: (for channel case insensetive fix)

* MODE reply. (Done)
* TOPIC reply. (Doen)
* KICK broadcast. (Done)
* Invitation Msg. (Done)
* JOIN 0 broadcast. (Done)
* Join welcome msg. (Done)
* JOIN broadcast join to members. (Done)
* private msg to a channel broadcast. (Done)
* JOIN channel bug: (Done)

```
ircserv 332 Xoris :Welcome to #Channel
JOIN #Channel
ircserv 462 Xoris :ERROR: JOIN Client is already a member of this channel !
JOIN #channel
ircserv 332 Xoris :Welcome to #channel
```

issue with old code to inspect later:

```
    std::map<std::string, Channel>::iterator it = channels.find(nameChannel);
    if (it == channels.end())
    {
        if (!key.empty())
        {
            sendServerReply(fd, 999, "Error: Malformed JOIN argument, to set a key for a channel, use MODE #example +k");
            return ;
        }
        if (nameChannel.size() > 50)
        {
            sendServerReply(fd, 999, "Error: Channel's name does not comply with the server's rules !");
            return ;
        }
        std::string originalName = nameChannel;
        lowerChannelName(nameChannel);
        Channel newChannel(nameChannel, fd, originalName);
        std::set<int> ops = newChannel.getOperators();
        this->channels.insert(std::make_pair(nameChannel, newChannel));
        sendChanneljoin(fd, newChannel);
        return ;
    }
```

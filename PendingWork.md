## Pending work

### Done — success replies:

* Registration welcome sequence (001-004) — design just settled, not yet written (Done)
* Nick-change confirmation — same, design settled, not written (Done)
* JOIN success sequence (332/331 topic, 353/366 member list) (Done)
* INVITE success (341 RPL_INVITING) (Done)

### Working On:

* Universal channel-broadcast function (JOIN confirmation to channel (Done), KICK notification, JOIN 0/PART notification (Done))
* KICK's comment argument — parsed but unused, needs wiring into its eventual broadcast
* TOPIC message to the sender of the command
* Need to check for channel re JOINING
* Signals handling
* Clean fds once the server gets CTRL + C or other ending signals.
* Hostname placeholder decision ("localhost" vs real IP) (To leave as is)
* Channel name case-insensitivity fix
* PRIVMSG embedded \r\n injection check
* Real reference client testing (WeeChat/irssi/HexChat) — haven't touched this at all yet, and it's arguably the most important remaining item since it validates everything else
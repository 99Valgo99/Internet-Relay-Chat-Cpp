## Pending work

### Done — success replies:

* Registration welcome sequence (001-004) — design just settled, not yet written (Done)
* Nick-change confirmation — same, design settled, not written (Done)
* JOIN success sequence (332/331 topic, 353/366 member list) (Done)
* INVITE success (341 RPL_INVITING) (Done)

### Working On:

* Universal channel-broadcast function (JOIN confirmation to channel (Done), KICK notification (Done), JOIN 0/PART notification (Done))
* KICK's comment argument — parsed but unused, needs wiring into its eventual broadcast (Done)
* TOPIC message to the sender of the command (Done)
* Need to check for channel re JOINING (Done)
* flushing Partial commands using nc -C + CTRL + D test from subject (Done).
* in case the operator has left the channel and no op left, the first client joined after the operator and not an op becomes an op. (Done)
* Signals handling && Clean fds once the server gets CTRL + C or other ending signals. (Done)
* Hostname placeholder decision ("localhost" vs real IP) (Done)
* Channel name case-insensitivity fix
* Nickname character-set/length grammar validation.
* PRIVMSG embedded \r\n injection check
* Real reference client testing (WeeChat/irssi/HexChat) — haven't touched this at all yet, and it's arguably the most important remaining item since it validates everything else
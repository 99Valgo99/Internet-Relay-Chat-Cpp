### WeeChat

#### Connect WeeChat to the ircserv:

```
run the server first:
    ./ircserv ip:port <password>

run WeeChat:
    WeeChat (on the cli)

run:
    /server add <name> ip/port -password=<password>

run:
    /connect <name>
```

WeeChat will attempt to establish a TLS/SSL encrypted connection, and since our ircserv runs on unencrypted TCP connection, deactivate the TLS conncetion feature.

```
/set irc.server.<name>.tls off
/disconnect <name>
/connect <name>
```
***

### Testing Our IRC Server With WeeChat

#### Piece of code to maintain PING/PONG connection

```
            else if (command == "PING")
            {
                std::string token;
                std::getline(stream, token);
                if (!token.empty() && token[0] == ' ')
                    token.erase(0, 1);
                clients[fd].sendBytes.append("PONG " + token + "\r\n");
            }
```

* Testing PRIVMSG: (Done)

found an error bug: :ircserv 401 Bob :Error: No such a client with the nickname: (no nickname provided) (fixed)

```
/msg Bob Hey sir !

:ircserv 001 Bob :Welcome to the internet relay network chat Bob!x@localhost
:xoris!xoris@localhost PRIVMSG Bob :Hey sir !
```

```
/msg #general hello guys
JOIN #general
:ircserv 332 Bob :Welcome to #general
:xoris!xoris@localhost PRIVMSG #general :hello guys
```


* Testing JOIN (Done)

* Testing INVITE (Done)

* Testing KICK (Done)

* 


### WeeChat Testing

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

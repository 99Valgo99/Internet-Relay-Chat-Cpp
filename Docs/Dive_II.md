# DIVE II

![Logo](../Img/IRC.png)
***

### From nothing to a listening socket

#### Tools:

``socket(AF_INET, SOCK_STREAM, 0)``

* ``AF_INET`` is the address family, meaning IPv4, it tells the kernel what kind of addresses this socket deals with.

* ``SOCK_STREAM`` is the scoket type, meaning TCP byte stream, this is the parameter that makes the socket TCP.

* ``0`` is the protocol, passing ``0`` lets the kernel pick the right protocl for that family and type, which is TCP here.

* it returns an fd, whcich becomes ``listen_fd``. it return ``-1`` on failure.
***

``setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))``

* ``listen_fd`` is the socket whose options we are changing.

* ``SOL_SOCKET`` is the level, meaning the option belongs to the generic socket layer rathern than to TCP specifically.

* ``SO_REUSADDR`` is the option itself, after a server stops, the OS keeps its port in ``TIME_WAIT`` for a while, without this option, restarting right away fails on ``bind()`` with "***address is already in use***".

* ``&opt`` points to an ``int`` set to ``1``, which means turn the option's on.

* ``sizeof(opt)`` is the size of that value, so the kernel knows how many bytes to read.
***

``struct sockaddr_in address``

* This struct describes the local address we want to bind to. We zero it with ``memset`` first so no field holds garbage.

* ``sin_family = AF_INET`` must match the family passed to ``socket()``
* ``sin_port = htons(port)`` is the port. ``htons`` converts it from host byte order to network byte order (big-endian). Without it, the port would e interpreted wrongly on little-endian machines and we would bind to a different port than intended.
* ``sin_addr.s_addr = INADDR_ANY`` means listen on every network interface of the machine, so clients can reach us however they connect.
***

``bind(listen_fd, (sockaddr*)&address, sizeof(address))``

* ``listen_fd`` is the socket to attach the address to.
* ``(sockaddr*)&address`` is our ``sockaddr_in`` cast to the generic ``sockaddr*`` type. 
* ``sizeof(address)`` is the struct size, so the kernel knows how much to read.
* This call claims the port for our process. From now on, traffic to that port is routed on our socket ``listen_fd``.
***

``listen(listen_fd, SOMAXCONN)``

* ``listen_fd`` is the bound socket to switch into passive mode, where it accepts connections instead of initiating them.
* ``SOMAXCONN`` is the backlog size, the maximum number of fully handshaken connections the kernel will quere while we haven't called ``accept()`` yet. ``SOMAXCONN`` is the system's maximum, so a brust of simultaneous connections isn't dropped.
***

``fcntl(listen_fd, F_SETFL, O_NONBLOCK)``

* ``listen_fd`` is the fd to modify.
* ``F_SETFL`` set the status flag.
* ``O_NONBLOCK`` is the flag being set. From now on, operations on this fd return immediately instead of sleeping when they can't complete.
***

``accept(listen_fd, NULL, NULL)``

* ``listen_fd`` is the listening socket to take a connection from.
* The second argument is where ``accept()`` would write the client's address. WE pass ``NULL`` because we don't need it. The hostname in our replies is hardcoded to ``localhost``.
* The third argument is the size of that address buffer, and its ``NULL`` for the same reason.
***

``recv(fd, tempo, sizeof(tempo), 0)``

* fd is the client socket to read from
* ``tempo`` is a ``char[1024]`` on the stack. it is scratch space for this one call, not where message live.
* ``sizeof(tempo)`` is the most bytes ``recv()`` may write into ``tempo``. it is an upper limit not a promse. We get whatever is available, up to 1024.
* ``0`` is a flag argument, we need no special behaviour.
* it returns the number of bytes read, ``0``, or ``-1``.
***


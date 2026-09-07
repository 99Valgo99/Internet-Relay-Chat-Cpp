# Internet Relay Chat (C++)

### Key Concepts To Understand In Order To Go Through This Project

* 1. **TCP sockets**
* 2. **Blocking vs non-blocking I/O**
* 3. **``poll()`` itself**
* 4. **TCP as a byte stream, not a message stream**
* 5. **Why ``errno`` is off-limits for control flow**
* 6. **The IRC protocol shape**
* 7. **Project architecture**

## TCP Sockets

A **socket** is just a file descriptor (an ``int``) that the kernel treats specially -- we can ``read``/``write``/``poll`` it like a file, but it's actually a handle to a network connection or listening endpoint.

#### Server-side setup, in order:

```
socket()
setsocketopt()
bind()
listen()
accept()
```

``socket(domain, type, protocol)``

* ``domain:`` ``AF_INET`` = IPV4, ``AF_INET6`` == IPV6. (We will use IPV4)
* ``type``: ``SOCK_STEAM`` -- this is what makes it TCP, the other common option, ``SOCK_DGRAM`` which is UDP.
* ``protocol``: we pass 0 -- lets the kernel pick te right protocol for the domain+type combo (TCP in our case)
* Returns an ``int`` fd, or ``-1`` on error

``setsocketopt(fd, level, optname, optval, optlen)``

* ``level``: ``SOL_SOCKET`` -- means "this option is generic socket-layer, not protocol-specific"
* ``optname``: ``SO_REUSEADDR`` this one we need, without it: after our server closes, the OS holds the port in a ``TIME_WAIT`` state for a bit (leftover packet cleanup), and a frech ``bind()`` to that same port fails with "Address is already in use". Painful during dev when we are restarting our server every 30s, thus ``SO_REUSEADDR`` tells the kernel "let me rebind to this port anyway"
* ``optval``: pointer to an ``int`` set to ``1``
* ``optlen``: ``sizeof(int)``

``bind(fd, addr, addrlen)``

* ``addr``: a ``struct sockaddr_in`` (for IPv4) cast to ``struct sockaddr*``. this struct holds:

-> ``sin_family`` = AF_INET (must match what we passed to socket())

-> ``sin_port``, our port number, but run through ``htons()`` first (host-to-network short which convert our ``int`` from machine's byte order to network byte order, without this our port might bind to a garbage port).

-> ``sin_addr`` which local ip to listen on. ``INADDR_ANY`` (0.0.0.0) means any interface on this machine, so the refrenece client can connect via ``localhost`` or the machine's LAN IP.

this is the call that actually claims "port xxxx belongs to my process now.

``listen(fd, backlog)``

* ``backlog`` max number of fully-connceted-but-not-yet-accepted clients, the kernel will queue for us. If a burt of clients connects before we call ``accept()``, the kernel holds them here rather than dropping them, a modest number (``SOMAXCONN`` or just ``10``) is fine.

This is the call that flips the fd from "just bound" to "actually listening for incoming TCP handshakes"

``accept(fd, addr, addrlen)``

* Pulls one connection off that backlog queue and gives us a new, seperate fd represeting that one client's socket. The original listening fd keeps listening -- it's never used for actual data transfer.
* ``addr``/``addrlen`` are optional out-params if we want the client's IP/port. (not required by the subject, but harmless if we want it for logging)
* returns ``-1`` on error, or (in non-blocking mode) ``-1`` with ``errno == EAGAIN/EWOULDBLOCK`` if no connection is pending -- but we are not allowed to branch on that, we only call ``accept()`` when ``poll()`` already told us the listening fd is readable.
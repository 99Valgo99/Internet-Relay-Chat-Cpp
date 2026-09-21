# Internet Relay Chat (C++)

![logo](Img/IRC.png)

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

## Blocking vs Non-blocking I/O

**Default behavior (blocking)**: when we call accept(), recv() or send() on a normal socket, if the operation can't complete immediately, the calling thread sleeps until it can. ``accept()`` blocks until a client connects. ``recv()`` blocks until data arrives. ``send()`` blocks if the kernel's send buffer is full.

**Why that's fatal for this project**: we have exactly one thread and one ``poll()`` loop handling every client. If we call a blocking ``recv()`` on client A and client A hasn't sent anything yet, our entire server freezes -- client B,C and D can't be served, the listening socket can't ``accept()`` new connections, nothing happens -- until A finally sends something. That's the forking/threading problem in disguise: the subject bans forking, so blocking calls would single-handedly stall everyone.

**The fix**: ``O_NONBLOCK`` we set this flag on every fd (listening sokcet and every client socket) right after creating it:

```
fcntl(fd, F_SETFL, O_NONBLOCK);
```

**What changes at the kernel level**: the call no longer sleeps, if the operation can't complete right now, it returns immediately with ``-1`` and sets ``errno`` to ``EAGAIN`` (or ``EWOULDBLOCK``), same thing on most systems -- meaning "nothing to do right now, try again later". If data is available/the socket is ready, it behaves exactly like the blocking version and returns the data/result normally.

**The trap this sets up**: now that every call can return "Nothing happened yet", we need some way to know when to actually call ``recv()``/``accept()``/``send()`` instead of just calling them in a loop and checking ``errno == EAGAIN`` -- because that's the literal instant-zero rule ("never use errno to decide control flow after I/O"). Polling in a tight loop like that would also burn 100% CPU for no reason.

Thus we never call these blindly, we ask the kernel in advance, for a whole batch of fds at once, "which of these are actually ready right now?" that's exactly what ``poll()`` does, and why it exists as the load-bearing piece of this whole architecture.

## ``poll()`` -- The Core of The Server

**The problem is solves**: we have N fds (1 listening socket + 1 per connected client), all non-blocking. We can't just loop over them calling ``recv()`` on each -- most calls would return ``EAGAIN`` (nothing to read), wasting CPU, and we are not allowed to use ``errno`` for control flow anyway. We need the kernel to tell us, in one shot, which fds actually have something to do.

**The Signature**
```
int poll(struct pollfd *fds, nfds_t, int timeout);
```

* ``fds`` -- an array of ``struct pollfd``, one entry per fd we are watching
* ``nfds`` -- how many entries are in that array
* ``timeout`` -- miliseconds to wait before giving up if nothing's ready (``-1`` = block forever until something happens, which is normally whay we want for a server with nothing esle to do meanwhile.)
* Return Value: number of fds with evernts ready, ``0`` if the timeout expired, ``-1`` on error.

**The ``struct pollfd``:

```
struct pollfd {
    int fd; // the file descriptor to watch
    short events; // what we are asking about (input)
    short revents; // what actually happned (output, filled by poll())
};
```

* ``events`` -- we set this ourselves before calling ``poll()``. Common Flags:

-> ``POLLIN`` -- "tell me if this fd has data ready to read" (for a listening socket, this means a new connection is pending; for a client socket, it means data arrived)

-> ``POLLOUT`` "tell me if this fd is ready to accept a write without blocking" (we only need to watch for this when we actually have queued data to )

## Registration Overview

A client must complete three steps before the server treats it as fully registered and allows any other command (JOIN, PRIVMSG...):

* ``PASS <password>`` -- correct connection password
* ``NICK <nickname>`` -- a valid, unique nickname
* ``USER <username> <mode> <unused> :<realname>`` -- user identity info

Order between NICK and USER is flexible in real IRC clients (some send NICK first, some USER first), but **PASS should come before both** -- if a cleint tries NICK/USER before a correct PASS, the server should reject it. Registration is only "complete" once all three have succeded at least once.

### Pass

**Purpose**: authenticate the connection against the server's startup password

**Format**: ``PASS <password>``

#### Server behavior:

* Compare the given password to the one passed as ``argv[2]`` at server startup
* If correct: mark this client as "password accepted" internally
* If incorrect: the client should not be allowed to complete registration -- real servers send back an error numberic reply (``464 ERR_PASSWDMISMATCH``) and typically disconnect the client
* If a client sends PASS after already being fully registered: real server send ``462 ERR_ALREADYREGISTRED`` -- this is invalid at that point, not a way to change password mid-session

***

### NICK

**Purpose**: set/change the client's nickname -- their visible short identity

**Format**: ``NICK <nickname>``

#### Server behavior:

* Check the requested nickname isn't already in use by another connected client (nickname are unique server-wide)
* If available: assign it to this client
* If taken: reject with ``433 ERR_NICKNAMEINUSE``, client keeps whatever nick (or lack of one) if had before
* If missing entirely (e.g just ``NICK`` with no argument): ``431 ERR_NONICKNAMEGIVEN``

***

### USER

**Purpose**: set the client's username and realname (display identity info)

**Format**: ``USER <username> <mode> <unused> :<realname>``

#### Server behavior:

* Parse and store ``username`` and ``realname``
* ``<mode>`` and ``<unused>`` -- accept and discard; no mandatory-scope logic depends on them
* If a client sends USER after already being fully registered: ``462 ERR_ALREADYREGISTRED`` -- same as re-sending PASS, this ins't a way to update identity later
***

### Channel - the New Concept

A ``Channel`` doesn't exist until someone ``JOIN``s it -- there\s no pre-definde list of valid channels, they're created on demand and (in most real IRC servers) destroyed once empty. Each channel needs:

* **A name** (e.g ``#general``) -- channel names in IRC conventionally start with ``#``
* **A memeber list** -- who's currenlty joined
* **A topic**
* **An operator list** -- who has elevated privileges in this channel
* **Mode state**
***

### JOIN

**Purpose**: add the sending client to a channel's member list, creating the channel if it doesn't exist yet.

**Format**: ``JOIN <channel>`` (real IRC supports joining multiple channels in one command and channel keys)

#### Server behavior:

* Client must be registered
* Look up the channel by name, if it doesn't exist, it must be created
* Add the client's fd to the channel's member list
* Real IRC sends the joining client (and existing members) some numeric replies confirming the join and listing current members

### PRIVMSG

**Purpose**: send a message either directly to another user (by nickname) or to every member of a channel.

**Format**: ``PRIVMSG <target> :<message>``

**Server behavior** -- two distinct cases based on what ``<target>`` looks like:

1. **Target is nickname** -- look up which fd currently owns that nickname, forward the message directly to just that one client
2. **Target is a channel name** (starts with ``#``) -- look up the channel, forward the message to every member except the **sender**
***

### The Broadcast Mechanic

When forwarding a message to multiple members, we are calling ``send()`` on eah of their fds -- but remember, ``send()`` **can partailly write**, and we are not supposed to call ``send()``/``recv()`` outisde a single ``poll()`` loop's control. This is where the ``POLLOUT``/outbox conecpt fundamentals finally becomes unavoidable -- we can't keep deferring it once we have actively broadcasting to multiple clients per command.

***

### Structue shape of the Host Server

```
POLLIN  — direction: something → SERVER (incoming, server reads)

  listen_fd  : a new client is trying to connect (accept() time)
  client fd  : that client sent bytes (recv() time — commands, PRIVMSG text, etc.)

POLLOUT — direction: SERVER → something (outgoing, server writes)

  listen_fd  : never used — listener never sends data
  client fd  : server has bytes queued in that client's sendBytes,
               waiting to be delivered (send() time) — regardless of
               WHY those bytes exist:
                 - a relayed PRIVMSG from another client
                 - a channel broadcast
                 - a numeric error/reply the server itself generated
                 - a welcome message

Rule of thumb:
  POLLIN  = "can I read from this fd right now?"
  POLLOUT = "can I write to this fd right now?"

  Content/purpose of the data is irrelevant to which flag applies —
  only the DIRECTION of travel (into the server, or out of it) decides.
```

### MODE - Flags

```
_________________________________________________________
Flag | Meaning              | + needs arg? | - needs arg?
-----|----------------------|--------------|-------------
i    | invite-only          | no           | no
t    | topic-restricted     | no           | no
k    | channel key          | yes          | yes
o    | grant/revoke operator| yes          | yes
l    | user limit           | yes          | no
_________________________________________________________
```

### Server's Signal Handling:

When capturing system signals like ``CTRL + c`` (``SIGINT``) in C++, we cannot simply point the operating system to a standard member function, the OS requires a function with a specific C-style signature that can be called globally without needing an object instance.
We can also handle CTRL + Z from suspending our program and keep it in a frozen state, using two Macros ``SIGSTP`` in order to tell the OS what signal we are catching (mainly here ``Ctrl + Z``) then as handler we use the macro ``SIG_IGN`` that basically tells the OS to just ignore this signal and keep our process running.

#### The solution: Static Members (``static``)

To bridge C-style OS signals with an object-orented C++ class, we must use **Static** components

**Why Static ?**:

* **Static Variable**: acts as a shared, class-wide flag, because it is static, there is only one instance of it shared across the entire program, independent of whether any ``Server`` objects have been created, thus we must also declare it at the top of the ``.cpp`` file in order for the allocation to happen, (No object instance, no Automatic allocation occurs as well).

* **Static Function**: Belongs to the class blueprint rather than any specific object.

#### Why we use ``volatile``

``volatile``: because a signal handler is called ``asynchronously`` by the operating system (iterrupting our program out of nowhere), the compiler doesn't realize that our variable can change behind its back; thus:

* Without ``volatile``, the compiler might optimize our usage of it by caching the variable in a CPU register.
* Once cached, our program will never check the actual memory again, meaning it will completely ignore ``CTLR + c`` , ``volatile`` forces the compiler to re-read the variable from memory every single time.

#### Good notes about this section:

* **Linker vs. Runtime:** Static varibales must be defined in a ``.cpp`` file. Defining them inside a constructor fails because contructors run at runtime, whereas the linker allocates static memmory before the program even starts.

* **Private Access:** A ``static`` member function has full access to ``privae`` static variables of its class, even though external code cannot touch them directly.

* Regular functions & statics: regular non-static functions can read/write static variables, but cannot be used as OS signal handlers because they require an object-bound ``this`` pointer.
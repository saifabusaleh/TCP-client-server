# TCP-client-server
#### TCP client server C program with sockets ####


#### Features: ####

* Added client max fuctionallity, this mean specific number of clients can connect to a server(constant), if more clients try to connect they
will get msg_nack.

* Two clients with the same name can't connect to the server, the second one will get msg_nack.

* Client can only perform PEER(to connect to other client), only after he did MSG_WHO, otherwise he will get error msg

* Client can't perform PEER to himself(can't talk to himself), if he try to talk to himself he will get error msg.

* Every connection message from the client is performed in new socket, and then closed in the end.

* In peer to peer, when client receive connection, he can agree the connection and also can refuse it.


#### Usage: ####

* MSG_UP will get called automatically when launching the client, and it will open socket with listen with the specific port that received
from the server.

* MSG_WHO -- gets list of the connected peers(including the client that did the command).

* PEER -- to connect to other client.


#### Run Example ###
><instance1>:
>./chats

><instance2>:
>sefabu@cs:~/Linux-hw4/new$ ./chatc 1
>Socket created
>Connected

>Commands: 
> MSG_DOWN -- to disconnect from the client 
>MSG_WHO - to get connected clients list 
>PEER - to connect to another client 

>:Socket created
>MSG_WHO
>msgwho sent to client, waiting for res
>number of connected clients: 1 
>[0] name: 1 port: 13261
>Commands: 
>MSG_DOWN -- to disconnect from the client 
>MSG_WHO - to get connected clients list 
>PEER - to connect to another client 

>:MSG_DOWN
>Client has been successfully unregistered, closing..
>sefabu@cs:~/Linux-hw4/new$ ./chatc 1
>Socket created
>Connected

>Commands: 
>MSG_DOWN -- to disconnect from the client 
>MSG_WHO - to get connected clients list 
>PEER - to connect to another client 

>:Socket created
>PEER
>can't use PEER functionallity, need to do MSG_WHO first, and to connect depending on client number
>Commands: 
>MSG_DOWN -- to disconnect from the client 
>MSG_WHO - to get connected clients list 
>PEER - to connect to another client 

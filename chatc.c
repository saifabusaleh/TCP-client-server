  /*
   *    C client 
   * 		all right reserved to Saif.
   */
  #include<stdio.h> //printf
  #include<stdio.h> //printf
  #include<string.h>    //strlen
  #include<sys/socket.h>    //socket
  #include<arpa/inet.h> //inet_addr
  #include "chat.h"
  #define CLIENTS_MAX 10
  #define TRUE 1
  #define FALSE 0
  int my_port=0;// the port of the current client, initalized when MSG_UP returns
  int is_in_conn=0;// flag that is true if the current client is in connection(peer to peer_) with other client
  typedef struct client_info
  {
    char m_name[C_NAME_LEN + 1];
    in_addr_t address;
    in_port_t port;
  } client_info_t; // temporary struct that saves the client info in at after doing MSG_WHO
  
  void *connection_handler();
  void getFromInputAndSendToPeer(int peer_sock);
  int main(int argc , char *argv[])
  {
    client_info_t client_info[CLIENTS_MAX];
    int sock,i;
    int number_of_connected_clients=0;//number of connected clients after doing msg_who
    int is_msg_who_passed = FALSE; // boolean that telling if we already did msg_who or not
    struct sockaddr_in client;
    msg_type_t id;
    char message[C_BUFF_SIZE];
    if(argc != 2) {
      printf("you must write the desired client name, example ./client terminator\n");
      return;
    } 
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
      printf("Could not create socket");
    }
    puts("Socket created");
    
    client.sin_addr.s_addr = inet_addr("127.0.0.1");
    client.sin_family = AF_INET;
    client.sin_port = htons( C_SRV_PORT );
    
    //Connect to remote client
    if (connect(sock , (struct sockaddr *)&client , 
      sizeof(client)) < 0)
    {
      perror("connect failed. Error");
      return 1;
    }
    
    puts("Connected\n");
    msg_up_t msgUP;
    msgUP.m_type = MSG_UP;
    strcpy(msgUP.m_name, argv[1]);
    
    //Send msgUP, to register the current client in the server
    if( send(sock ,&msgUP , sizeof(msgUP) , 0) < 0)
    {
      puts("Send failed");
      return 1;
    }
    msg_ack_t resp;
    //Receive a reply
    int read_size = recv(sock,&id,sizeof(int),MSG_PEEK);
    if(read_size<=0) {
      printf("recv failed \n");
      close(sock);
      return 0;
    }
    
    if(id == MSG_ACK) {
      if( recv(sock , &resp , sizeof(msg_ack_t) , 0) < 0)
      {
	puts("recv failed");
	close(sock);
	return 0;
      }
      my_port = resp.m_port;
      pthread_t thread;
      
      if( pthread_create( &thread , NULL ,  connection_handler) < 0)
      {
	perror("could not create thread");
	close(sock);
	return 1;
      }
    } else if(id == MSG_NACK) {
      printf("received msg_nack from the server, exiting.. \n");
      close(sock);
      return 0;
    }
    //close the socket that use for msg_up
    close(sock);
    
    while(1)
    {
      if(is_in_conn == 1) continue;
      printf("Commands: \n MSG_DOWN -- to disconnect from the client \n MSG_WHO - to get connected clients list \n PEER - to connect to another client \n\n:");
      scanf("%s",message);
      if(strcmp(message,"MSG_WHO")==0){
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
	  printf("Could not create socket");
	  close(sock);
	  return 1;
	}
	puts("Socket created");
	//Connect to remote client
	if (connect(sock , (struct sockaddr *)&client , 
	  sizeof(client)) < 0)
	{
	  perror("connect failed. Error");
	  close(sock);
	  return 1;
	}
	is_msg_who_passed = TRUE;
	msg_who_t msgwho;
	msgwho.m_type = MSG_WHO;
	if( send(sock ,&msgwho , sizeof(msgwho) , 0) < 0)
	{
	  puts("Send failed");
	  return 1;
	}
	msg_hdr_t msgHDR;
	printf("msgwho sent to client, waiting for res\n");
	if( recv(sock , &msgHDR , sizeof(msgHDR) , 0) < 0)
	{
	  puts("recv failed");
	  close(sock);
	  break;
	}
	printf("number of connected clients: %d \n",msgHDR.m_count);
	number_of_connected_clients = msgHDR.m_count;
	msg_peer_t msgPEER;
	for(i=0;i<msgHDR.m_count;i++) {
	  if( recv(sock , &msgPEER , sizeof(msgPEER) , 0) < 0)
	  {
	    puts("recv failed");
	    	  close(sock);
	    break;
	  }
	  client_info[i].address = inet_addr("127.0.0.1");
	  client_info[i].port = msgPEER.m_port; 
	  strcpy(client_info[i].m_name,msgPEER.m_name); 
	  printf("[%d] name: %s port: %d\n",i,msgPEER.m_name,msgPEER.m_port);
	}
	//close the socket of msg_who
        close(sock);
	continue;
      }
      if(strcmp(message,"MSG_DOWN")==0) {
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
	  printf("Could not create socket");
	  close(sock);
	  return 1;
	}
	puts("Socket created");
	//Connect to remote client
	if (connect(sock , (struct sockaddr *)&client , 
	  sizeof(client)) < 0)
	{
	  perror("connect failed. Error");
	  close(sock);
	  return 1;
	}
	msg_down_t msgDOWN;
	msgDOWN.m_type = MSG_DOWN;
	msgDOWN.m_port = my_port;
	msgDOWN.m_addr = inet_addr("127.0.0.1");
	if( send(sock ,&msgDOWN , sizeof(msgDOWN) , 0) < 0)
	{
	  puts("Send failed");
	  return 1;
	}
	printf("Client has been successfully unregistered, closing..\n");
	//close the socket of msg_down
	close(sock);
	return 0;
      }
      
      if(strcmp(message,"PEER")==0) { //C<Client_num>
	if(is_msg_who_passed == FALSE) {
	  printf("can't use PEER functionallity, need to do MSG_WHO first, and to connect depending on client number\n");
	  continue;
	}
	printf("enter peer to connect to, example: 0, will connect to c lient number 0\n");
	int client_num;
	scanf("%d",&client_num);
	if(client_num +1 > number_of_connected_clients) {
	  printf("cant connect to %d, because client number doesn't exist\n", client_num);
	  continue;
	}
	if(strcmp(client_info[client_num].m_name,argv[1]) == 0) //client can't talk to himself
	{
	  printf("client can't talk to himself!!!!, choose another client\n");
	  continue;
	}
	//Connect to client number client_num
	//printf(" port: %d\n",client_info[client_num].port);
	int peer_sock;
	struct sockaddr_in client_peer;
	peer_sock = socket(AF_INET , SOCK_STREAM , 0);
	if (peer_sock == -1)
	{
	  printf("Could not create socket");
	}
	puts("Socket created");
	
	client_peer.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_peer.sin_family = AF_INET;
	client_peer.sin_port = htons( client_info[client_num].port);
	
	//Connect to remote client
	if (connect(peer_sock , (struct sockaddr *)&client_peer , 
	  sizeof(client_peer)) < 0)
	{
	  perror("connect failed. Error");
	  return 1;
	}
	puts("Connected\n");
	msg_conn_t msgCONN;
	msgCONN.m_type = MSG_CONN;
	msgCONN.m_addr = inet_addr("127.0.0.1");//TODO: change this
	msgCONN.m_port = client_info[client_num].port;
	strcpy(msgCONN.m_name,client_info[client_num].m_name);
	if( send(peer_sock ,&msgCONN , sizeof(msgCONN) , 0) < 0)
	{
	  puts("Send failed");
	  return 1;
	}
	msg_resp_t msgRESP;
	if( recv(peer_sock , &msgRESP , sizeof(msgRESP) , 0) < 0)
	{
	  puts("recv failed");
	  break;
	}
	if(msgRESP.m_agree == 1) {
	  msg_text_t msgTEXT;
	  printf("the user agreed\n, send text to send to user, enter quit to close connection\n");
	  pthread_t thread;
	  
	  if( pthread_create( &thread , NULL ,  getFromInputAndSendToPeer,peer_sock) < 0)
	  {
	    perror("could not create thread");
	    return 1;
	  }
	  is_in_conn = 1;
	  while(is_in_conn == 1){
	    int read_size = recv(peer_sock,&id,sizeof(int),MSG_PEEK);
	    if(read_size<=0) {
	      printf("quit called, will close now and get back to main menu \n");
	      close(peer_sock);
	      break;
	    }
	    if(id == MSG_TEXT) {
	      read_size = recv(peer_sock,&msgTEXT,sizeof(msgTEXT),0);
	      if(read_size<=0)printf("inner recv failed");
	      printf("%s\n",msgTEXT.m_text);
	    }
	    else if( id == MSG_END) {
	      printf("closing connection\n");
	      is_in_conn = 0;
	      close(peer_sock);
	    }
	  }
	}
	continue;
      }
      //UNKOWN Command area
      printf("can't find your command, notice that the commands is CASE SENSETIVE!! \n");
    }
    
    printf("exiting the client..\n");
    return 0;
  }
  
  void *connection_handler()
  {
    int client_socket_desc, client_sock, read_size, ID;
    struct sockaddr_in client;
    msg_conn_t msgCONN;
    msg_text_t msgTEXT;
    int resp;
    msg_type_t id;
    //Create socket
    client_socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (client_socket_desc == -1)
    {
      printf("Could not create socket");
    }
    puts("Socket created");
    
    //Prepare the sockaddr_in structure
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = INADDR_ANY;
    client.sin_port = htons( my_port);
    
    //Bind
    if( bind(client_socket_desc,(struct sockaddr *)&client , sizeof(client)) < 0)
    {
      //print the error message
      perror("bind failed. Error");
      return 0;
    }
    
    //Listen
    listen(client_socket_desc , 3);
    
    while( (client_sock = accept(client_socket_desc, (struct sockaddr *)0, (socklen_t*)0)) )
    {
      puts("connection accepted\n");
      int new_sock = client_sock;
      read_size = recv(new_sock ,&ID , sizeof(int) , MSG_PEEK);
      if(ID == MSG_CONN) {
	is_in_conn = 1;
	read_size = recv(new_sock,&msgCONN,sizeof(msgCONN),0);
	printf("name: %s \n",msgCONN.m_name);
	msg_resp_t msgRESP;
	msgRESP.m_type = MSG_RESP;
	printf("\n you got connection, agree? write 1 or 0\n");
	scanf("%d",&resp);
	while(!(resp ==0 || resp == 1)) {
	  printf("you can enter or 0 or 1 only,try again..\n");
	  scanf("%d",&resp);
	}
	//send user response to the connected user
	msgRESP.m_agree = resp; 
	write(new_sock , &msgRESP , sizeof(msgRESP));
	if(resp == 0) // if user don't want to talk then terminate
	{
	  is_in_conn = 0;
	  close(new_sock);
	  continue;
	}
	//if user agreed then proceed
	pthread_t thread;
	if( pthread_create( &thread , NULL ,  getFromInputAndSendToPeer, new_sock) < 0)
	{
	  perror("could not create thread");
	  return 0;
	}
	
	
	while(is_in_conn ==1 ) {
	  
	  read_size = recv(new_sock,&id,sizeof(int),MSG_PEEK);
	  if(id == MSG_TEXT) {
	    read_size = recv(new_sock,&msgTEXT,sizeof(msgTEXT),0);
	    printf("%s\n",msgTEXT.m_text);
	  }
	  else if(id == MSG_END) {
	    printf("closing connection\n");
	    is_in_conn = 0;
	    close(new_sock);
	    return 0;
	  }       
	}
      }  
    }
    printf("exiting client thread\n");
    
    close(client_socket_desc);
    return 0;
  }
  
  void getFromInputAndSendToPeer(int peer_sock) {
    msg_text_t msgTEXT;
    char peer_message[C_BUFF_SIZE];
    while(1) {
      scanf("%s",peer_message);
      if(strcmp(peer_message,"quit")==0) {
	msg_end_t msgEND;
	msgEND.m_type = MSG_END;
	if( send(peer_sock ,&msgEND , sizeof(msgEND) , 0) < 0)
	{
	  puts("Send failed");
	  return;
	}
	printf("sending quits\n");
	is_in_conn = 0;
	return;
      }
      msgTEXT.m_type = MSG_TEXT;
      msgTEXT.m_size = strlen(peer_message);
      strcpy(msgTEXT.m_text,peer_message);
      if( send(peer_sock ,&msgTEXT , sizeof(msgTEXT) , 0) < 0)
      {
	puts("Send failed");
	return;
      }
    }
  }

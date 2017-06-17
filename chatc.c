/*
 *    C ECHO client example using sockets
 */
#include<stdio.h> //printf
/*
 *    C ECHO client example using sockets
 */
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include "chat.h" 
int my_port=0;
int main(int argc , char *argv[])
{
  int sock;
  struct sockaddr_in server;
  char message[1000] , server_reply[2000];
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
  
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_family = AF_INET;
  server.sin_port = htons( C_SRV_PORT + 1 );
  
  //Connect to remote server
  if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
  {
    perror("connect failed. Error");
    return 1;
  }
  
  puts("Connected\n");
  msg_up_t msgUP;
  msgUP.m_type = MSG_UP;
  strcpy(msgUP.m_name, argv[1]);
  
  msg_down_t msgDOWN;
  msgDOWN.m_type = MSG_DOWN;
  msgDOWN.m_addr = inet_addr( "127.0.0.1");
  msgDOWN.m_port = 2133;
  //Send msgUP
  if( send(sock ,&msgUP , 2000 , 0) < 0)
  {
    puts("Send failed");
    return 1;
  }
  msg_ack_t resp;
  int id;
  //Receive a reply from the server
  if( recv(sock , &resp , 2000 , 0) < 0)
  {
    puts("recv failed");
    return 0;
  }
  my_port = resp.m_port;
  printf("client %s, has been successfuly registers at server with port %d\n",argv[1],my_port);
  
  //keep communicating with server
  while(1)
  {
    printf("Enter message: ");
    scanf("%s",message);
    if(strcmp(message,"MSG_WHO")==0){
      msg_who_t msgwho;
      msgwho.m_type = MSG_WHO;
      if( send(sock ,&msgwho , 2000 , 0) < 0)
      {
	puts("Send failed");
	return 1;
      }
      msg_hdr_t msgHDR;
      printf("msgwho sent to server, waiting for res\n");
      if( recv(sock , &msgHDR , 2000 , 0) < 0)
      {
	puts("recv failed");
	break;
      }
      printf("number of connected clients: %d \n",msgHDR.m_count);
      int i;
      msg_peer_t msgPEER;
      for(i=0;i<msgHDR.m_count;i++) {
	if( recv(sock , &msgPEER , 2000 , 0) < 0)
	{
	  puts("recv failed");
	  break;
	}
	printf("name: %s port: %d\n",msgPEER.m_name,msgPEER.m_port);
      }
    }
    if(strcmp(message,"MSG_DOWN")==0) {
      msg_down_t msgDOWN;
      msgDOWN.m_type = MSG_DOWN;
      msgDOWN.m_port = my_port;
      msgDOWN.m_addr = inet_addr("127.0.0.1");
      if( send(sock ,&msgDOWN , 2000 , 0) < 0)
      {
	puts("Send failed");
	return 1;
      }
      printf("Client has been successfully unregistered, closing..\n");
      return 0;
    }
  }
  
  close(sock);
  return 0;
  
}


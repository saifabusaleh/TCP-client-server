/*
  *    C ECHO client 
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
int my_port=0;

typedef struct client_info
{
  char m_name[C_NAME_LEN + 1];
  in_addr_t address;
  in_port_t port;
} client_info_t;
int main(int argc , char *argv[])
{
  client_info_t client_info[CLIENTS_MAX];
  int sock,i;
  int number_of_connected_clients=0;//number of connected clients after doing msg_who
  int is_msg_who_passed = FALSE; // boolean that telling if we already did msg_who or not
  struct sockaddr_in server;
  char message[C_BUFF_SIZE] , server_reply[C_BUFF_SIZE];
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
  if (connect(sock , (struct sockaddr *)&server , 
  sizeof(server)) < 0)
  {
    perror("connect failed. Error");
    return 1;
  }
  
  puts("Connected\n");
  msg_up_t msgUP;
  msgUP.m_type = MSG_UP;
  strcpy(msgUP.m_name, argv[1]);
  
  //Send msgUP
  if( send(sock ,&msgUP , sizeof(msgUP) , 0) < 0)
  {
    puts("Send failed");
    return 1;
  }
  msg_ack_t resp;
  //Receive a reply from the server
  if( recv(sock , &resp , sizeof(msg_ack_t) , 0) < 0)
  {
    puts("recv failed");
    return 0;
  }
  my_port = resp.m_port;
  printf("client %s, has been successfuly registers at server with port %d\n",argv[1],my_port);
  
  //keep communicating with server
  while(1)
  {
    printf("Commands: \n MSG_DOWN -- to disconnect from the server \n MSG_WHO - to get connected clients list \n PEER - to connect to another client \n\n:");
    scanf("%s",message);
    if(strcmp(message,"MSG_WHO")==0){
      is_msg_who_passed = TRUE;
      msg_who_t msgwho;
      msgwho.m_type = MSG_WHO;
      if( send(sock ,&msgwho , sizeof(msgwho) , 0) < 0)
      {
        puts("Send failed");
        return 1;
      }
      msg_hdr_t msgHDR;
      printf("msgwho sent to server, waiting for res\n");
      if( recv(sock , &msgHDR , sizeof(msgHDR) , 0) < 0)
      {
        puts("recv failed");
        break;
      }
      printf("number of connected clients: %d \n",msgHDR.m_count);
      number_of_connected_clients = msgHDR.m_count;
      msg_peer_t msgPEER;
      for(i=0;i<msgHDR.m_count;i++) {
        if( recv(sock , &msgPEER , sizeof(msgPEER) , 0) < 0)
        {
          puts("recv failed");
          break;
        }
        client_info[i].address = inet_addr("127.0.0.1");
        client_info[i].port = msgPEER.m_port; //TODO: maybe add name?
        strcpy(client_info[i].m_name,msgPEER.m_name); 
        printf("[%d] name: %s port: %d\n",i,msgPEER.m_name,msgPEER.m_port);
      }
      continue;
    }
    if(strcmp(message,"MSG_DOWN")==0) {
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
      printf(" port: %d\n",client_info[client_num].port);
      continue;
      
    }
    //UNKOWN Command area
    printf("can't find your command, notice that the commands is CASE SENSETIVE!! \n");
  }
  
  printf("closing the socket, and exiting the client..\n");
  close(sock);
  return 0;
}


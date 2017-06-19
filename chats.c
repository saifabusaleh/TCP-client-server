/*
  *    C socket server
*/

#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include "chat.h"
#define CLIENTS_MAX 10
typedef struct clientStruct {
  in_port_t m_port;
  char m_name[C_NAME_LEN + 1];
} client;

//the thread function
void *connection_handler(void *);
client* clients;
int client_index=0;
int main(int argc , char *argv[])
{
  clients = (client *) malloc(sizeof(client)*CLIENTS_MAX);
  int server_socket_desc , client_sock , c , *new_sock,socket_desc;
  struct sockaddr_in server , client;
  
  //Create socket
  server_socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (server_socket_desc == -1)
  {
    printf("Could not create socket");
  }
  puts("Socket created");
  
  //Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons( C_SRV_PORT + 1 );
  
  //Bind
  if( bind(server_socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
  {
    //print the error message
    perror("bind failed. Error");
    return 1;
  }
  puts("bind done");
  
  //Listen
  listen(server_socket_desc , 3);
  
  //Accept and incoming connection
  puts("Waiting for incoming connections...");
  c = sizeof(struct sockaddr_in);
  while( (client_sock = accept(server_socket_desc, (struct sockaddr *)0, (socklen_t*)0)) )
  {
    puts("Connection accepted");
    
    pthread_t sniffer_thread;
    new_sock = malloc(1);
    *new_sock = client_sock;
    
    if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
    {
      perror("could not create thread");
      return 1;
    }
    
    //Now join the thread , so that we dont terminate before the thread
    //    pthread_join( sniffer_thread , NULL);
    puts(" thread  finished");
  }
  
  if (client_sock < 0)
  {
    perror("accept failed");
    return 1;
  }
  
  return 0;
}

/*
  * This will handle connection for each client
* */
void *connection_handler(void *server_socket_desc)
{
  //Get the socket descriptor
  int sock = *(int*)server_socket_desc;
  int read_size,i;
  char *message , client_message[C_BUFF_SIZE];
  msg_up_t msgUP;
  msg_down_t msgDOWN;
  msg_type_t ID;
  in_addr_t address;
  msg_hdr_t msgHEADER;
  msg_peer_t msgPEER;
  char name[C_BUFF_SIZE];
  while((read_size = recv(sock ,&ID , sizeof(int) , MSG_PEEK)) > 0) {
    switch(ID) {
      case MSG_DOWN: 
      read_size = recv(sock,&msgDOWN,sizeof(msgDOWN),0);
      if(read_size<=0)printf("recv for msgDOWN failed");
      int deleteIndex;
      for(i=0;i<client_index;i++){
        if(msgDOWN.m_port == clients[i].m_port){
          deleteIndex=i;
          break;
        }
      }
      client temp;
      for(i=deleteIndex;i<client_index-1;i++) {
        temp=  clients[i];
        clients[i] = clients[i+1];
        clients[i+1] = temp;
      }
      client_index--;
      printf("msgDOWN: server: client deleted\n");
      break;
      
      case MSG_UP:
      read_size = recv(sock , &msgUP , sizeof(msgUP) , 0);
      if(read_size<=0)printf("recv for msgUP failed"); 
      clients[client_index].m_port = C_SRV_PORT+client_index+1;
      strcpy(clients[client_index].m_name,  msgUP.m_name);
      msg_ack_t msgACK;
      msgACK.m_type = MSG_ACK;
      msgACK.m_port = clients[client_index++].m_port;
      write(sock , &msgACK , sizeof(msgACK));
      break;
      
      case MSG_WHO:
      msgHEADER.m_type = MSG_HDR;
      msgHEADER.m_count = client_index;
      write(sock,&msgHEADER,sizeof(msgHEADER));     
      for(i=0;i<client_index;i++) {
        msgPEER.m_type = MSG_PEER;
        //	   msgPEER.m_addr = clients[i].m_addr;
        msgPEER.m_port = clients[i].m_port;
        strcpy(msgPEER.m_name,clients[i].m_name);
        write(sock,&msgPEER,sizeof(msgPEER));
      } 
      break;
    }
    
  } 
  close(sock);
  free(server_socket_desc);
  
  return 0;
}

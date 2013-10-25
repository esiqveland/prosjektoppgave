/*
** talker.c -- a datagram "client" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT "32001"    // the port users will be connecting to
#define MAXBUFLEN 128

int main(int argc, char *argv[])
{
    int i,n;
    int sockfd;
    
    struct sockaddr_in load_dist_addr, my_addr, receiving_addr;
    socklen_t len;
    
    char mesg[MAXBUFLEN];

    printf("setup socket...\n");
    
    sockfd=socket(AF_INET,SOCK_DGRAM,0);

    //Receiving netinfo
    bzero(&receiving_addr,sizeof(receiving_addr));
    receiving_addr.sin_family = AF_INET;

    //My netinfo
    bzero(&my_addr,sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    my_addr.sin_port=htons(32005);;
    
    bzero(&load_dist_addr,sizeof(load_dist_addr));
    load_dist_addr.sin_family = AF_INET;
    load_dist_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    load_dist_addr.sin_port=htons(32000);;
    
    bind(sockfd,(struct sockaddr *)&my_addr,sizeof(my_addr));
    
    printf("sending message...\n");
    typedef struct {
        __uint32_t ip;
        __uint16_t port;
        char msg[MAXBUFLEN];
    } payload;

    payload p;
    
    p.ip = my_addr.sin_addr.s_addr;
    p.port = my_addr.sin_port;
    strcpy(p.msg,"wash car technolog ");
    
    sendto(sockfd,&p,sizeof(p),0,(struct sockaddr *)&load_dist_addr,sizeof(load_dist_addr));

    printf("-------------------------------------------------------\n");
    printf("sent query: %s\n",p.msg);
    printf("-------------------------------------------------------\n");

    printf("Ready for query...\n");
    bzero(&p,sizeof(p));

    char stringBuffer[1024];

    len = sizeof(receiving_addr);
        
    n = recvfrom(sockfd,stringBuffer,1024,0,(struct sockaddr *)&receiving_addr,&len);  

    printf("Received: %s\n",stringBuffer);


    close(sockfd);
    
}

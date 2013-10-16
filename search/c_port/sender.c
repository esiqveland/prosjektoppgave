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
    
    struct sockaddr_in servaddr, useraddr, cliaddr;
    socklen_t len;
    
    char mesg[MAXBUFLEN];

    printf("setup socket...\n");
    
    sockfd=socket(AF_INET,SOCK_DGRAM,0);

    //Client netinfo
    bzero(&useraddr,sizeof(useraddr));
    useraddr.sin_family = AF_INET;
    
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.102");
    servaddr.sin_port=htons(32001);;
    
    bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    
    printf("sending message...\n");
    typedef struct {
        __uint32_t ip;
        __uint16_t port;
        char msg[MAXBUFLEN];
    } payload;

    payload p;
    
    p.ip = servaddr.sin_addr.s_addr;
    p.port = servaddr.sin_port;
    strcpy(p.msg,"wash car technolog ");
    
    sendto(sockfd,&p,sizeof(p),0,(struct sockaddr *)&servaddr,sizeof(servaddr));

    printf("-------------------------------------------------------\n");
    printf("sent query: %s\n",p.msg);
    printf("-------------------------------------------------------\n");

    printf("Ready for query...\n");
    bzero(&p,sizeof(p));

    char stringBuffer[1024];

    len = sizeof(cliaddr);
        
    n = recvfrom(sockfd,stringBuffer,1024,0,(struct sockaddr *)&cliaddr,&len);  

    printf("Received: %s\n",stringBuffer);


    close(sockfd);
    
}

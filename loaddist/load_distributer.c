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
#include <time.h>

#define MAXBUFLEN 1024
#define NODE_IP_START_IP "192.168.0.100"
#define NODE_IP_START_IP_NUM 100
#define IP_LENGHT 8

void setup(){
    int sockfd;
    ssize_t n;
    
    struct sockaddr_in servaddr, nodeaddr, useraddr;
    socklen_t len;
    
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    
    //Load distributer netinfo
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.101"); //TODO: Pull ip from network interface
    servaddr.sin_port=htons(32002);
    
    //Client netinfo
    bzero(&useraddr,sizeof(useraddr));
    useraddr.sin_family = AF_INET;
    
    //Worker node netinfo
    bzero(&nodeaddr,sizeof(nodeaddr));
    nodeaddr.sin_family = AF_INET;
    nodeaddr.sin_port=htons(32003);
    
    char * node_address[8];
    
    node_address[0] = "192.168.0.200";
   	node_address[1] = "192.168.0.201";
   	node_address[2] = "192.168.0.202";
   	node_address[3] = "192.168.0.203";
   	node_address[4] = "192.168.0.204";
   	node_address[5] = "192.168.0.205";
   	node_address[6] = "192.168.0.206";
    node_address[7] = "192.168.0.207";
    
    bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    
    printf("starting server...\n");
    
    char temp_buff[1024];

    typedef struct {
        __uint32_t ip;
        __uint16_t port;
        char msg[MAXBUFLEN];
    } payload;
    
    payload p;

    srand(time(NULL));


    
    for(;;)
    {
        printf("Ready for query...\n");
        bzero(&p,sizeof(p));
        len = sizeof(useraddr);
        
        bzero(&temp_buff,(sizeof(temp_buff)));

        // int k;
        // unsigned int m = sizeof(k);
        
        // getsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(void *)&k, &m);
        // // now the variable n will have the socket size
        // printf("%d\n",k);
        // printf("%d\n",m);



        n = recvfrom(sockfd,&temp_buff,sizeof(temp_buff),0,(struct sockaddr *)&useraddr,&len);
        
        //useraddr.sin_addr.s_addr = p.ip;
        //useraddr.sin_port = p.port;

        p.ip = useraddr.sin_addr.s_addr;
        p.port = useraddr.sin_port;
        strcpy(p.msg, temp_buff);

        printf("received from user on port: %hu, ip: %s\n",ntohs(p.port),inet_ntoa(useraddr.sin_addr));
        printf("query: %s\n",p.msg);
        
        
        // We have a request, send to a working node
        
        // Select node

        int node = 0;
        nodeaddr.sin_addr.s_addr = inet_addr(node_address[0]);
        
        printf("Sending message to node number %d: %s\n",node, node_address[node]);
	printf("Message:\n%s\n", p.msg);
        sendto(sockfd,&p,sizeof(p),0,(struct sockaddr *)&nodeaddr,sizeof(nodeaddr));
    }
    close(sockfd);
        
    
}

int main(void)
{
    setup();
    return 0;
}

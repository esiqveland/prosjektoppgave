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
#define NODE_SERV_PORT 32002
#define CONFIG_TARGET_PORT 32003
#define NUM_TOTAL_NODES 7

void setup(){
    char* configfile = "config.txt";
    char configbuffer[16];

    FILE* fconfig = fopen(configfile, "r");
    fgets(configbuffer, 16, fconfig);

    printf("Starting server on ip: %s:%d\n", configbuffer, NODE_SERV_PORT);

    int sockfd;
    ssize_t n;

    struct sockaddr_in servaddr, nodeaddr, useraddr;
    socklen_t len;

    sockfd=socket(AF_INET,SOCK_DGRAM,0);

    //Load distributer netinfo
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(configbuffer); //TODO: Pull ip from network interface
    servaddr.sin_port=htons(NODE_SERV_PORT);

    //Client netinfo
    bzero(&useraddr,sizeof(useraddr));
    useraddr.sin_family = AF_INET;

    //Worker node netinfo
    bzero(&nodeaddr,sizeof(nodeaddr));
    nodeaddr.sin_family = AF_INET;
    nodeaddr.sin_port=htons(CONFIG_TARGET_PORT);

    char * node_address[NUM_TOTAL_NODES];

    node_address[0] = "192.168.0.200";
    node_address[1] = "192.168.0.201";
    node_address[2] = "192.168.0.202";
    node_address[3] = "192.168.0.203";
    //node_address[4] = "192.168.0.204";
    node_address[4] = "192.168.0.205";
    node_address[5] = "192.168.0.206";
    node_address[6] = "192.168.0.207"; // og 199

    // convert addresses beforehand
    in_addr_t nodes[NUM_TOTAL_NODES];

    int asdf = 0;
    for(asdf = 0; asdf < NUM_TOTAL_NODES; asdf++) {
        nodes[asdf] = inet_addr(node_address[asdf]);
    }

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

    unsigned int node_counter = 0;

    for(;;)
    {
        bzero(&p,sizeof(p));
        len = sizeof(useraddr);

        //bzero(&temp_buff,(sizeof(temp_buff)));

        n = recvfrom(sockfd,&p.msg,sizeof(p.msg),0,(struct sockaddr *)&useraddr,&len);

        p.ip = useraddr.sin_addr.s_addr;
        p.port = useraddr.sin_port;
        //strcpy(p.msg, temp_buff);

        //printf("received from user on port: %hu, ip: %s\n",ntohs(p.port),inet_ntoa(useraddr.sin_addr));
        //printf("query: %s\n",p.msg);

        // Select node
        int node = (node_counter % NUM_TOTAL_NODES-1) + 1;
        node_counter++;
        nodeaddr.sin_addr.s_addr = nodes[node];

        //printf("Sending message to node number %d: %s\n",node, node_address[node]);
        //printf("Message:\n%s\n", p.msg);
        sendto(sockfd,&p,sizeof(p),0,(struct sockaddr *)&nodeaddr,sizeof(nodeaddr));
    }
    close(sockfd);


}

int main(void)
{
    setup();
    return 0;
}

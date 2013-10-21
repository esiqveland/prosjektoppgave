
// /* Sample UDP server */

// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <stdio.h>

// #include <fcntl.h>



// int main(int argc, char**argv)
// {
//    int socket_in, socket_out,n;
//    struct sockaddr_in servaddr,cliaddr;
//    socklen_t len;
//    char mesg[1000];

//    socket_in = socket(AF_INET,SOCK_DGRAM,0);

//    bzero(&servaddr,sizeof(servaddr));

//    servaddr.sin_family = AF_INET;
//    servaddr.sin_addr.s_addr=htonl("192.168.0.102"); //input ip
//    servaddr.sin_port=htons(9999);
   
//    bind(socket_in,(struct sockaddr *)&servaddr,sizeof(servaddr));
   
//    for (;;)
//    {
//       len = sizeof(cliaddr);
//       n = recvfrom(socket_in,mesg,1000,0,(struct sockaddr *)&cliaddr,&len);
//       //sendto(socket_in,mesg,n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
//       mesg[n] = 0;
//       printf("Received the following:\n");
//       printf("%s",mesg);
//    }
// }


/*
** listener.c -- a datagram sockets "server" demo
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

#define MYPORT "4951"    // the port users will be connecting to

#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    printf("listener: waiting to recvfrom...\n");

    addr_len = sizeof their_addr;
    
    while(1==1)
    {
       if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
           (struct sockaddr *)&their_addr, &addr_len)) == -1) {
           perror("recvfrom");
           exit(1);
       }

       printf("listener: got packet from %s\n",
           inet_ntop(their_addr.ss_family,
               get_in_addr((struct sockaddr *)&their_addr),
               s, sizeof s));
       printf("listener: packet is %d bytes long\n", numbytes);
       buf[numbytes] = '\0';
       printf("listener: packet contains \"%s\"\n", buf);

       printf("executing query...\n");

       //DO stuff
   }
    close(sockfd);

    return 0;
}

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

#define MYPORT "4950"    // the port users will be connecting to
#define SERVERPORT "4951"

#define MAXBUFLEN 1024
#define NODE_COUNT 1
#define NODE_IP_START_IP "192.168.0.100"
#define NODE_IP_START_IP_NUM 100
#define IP_LENGHT 8


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int setAddressWithNodeNumber(int i, char* address)
{

	int j;
	for(j = 0;j < 8;j++)
	{
		address[j] = NODE_IP_START_IP[j];
	}

	int nodenumber = NODE_IP_START_IP_NUM + i;
	char str[4];
	sprintf(str, "%d", nodenumber);

	address[10] = str[0];
	address[11] = str[1];
	address[12] = str[2];
	address[13] = str[3];

	return 0;
}

int main(void)
{
	//setup receiving socket:
    int rec_sock,send_sock[NODE_COUNT];
    struct addrinfo rec_hints, *rec_servinfo, *rec_p;
    struct addrinfo send_hints[NODE_COUNT], *send_servinfo[NODE_COUNT], *send_p[NODE_COUNT];
    int rec_rv, send_rv[NODE_COUNT];
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    //---------------  setup receiving socket  ----------------------------------//

    memset(&rec_hints, 0, sizeof rec_hints);
    rec_hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    rec_hints.ai_socktype = SOCK_DGRAM;
    rec_hints.ai_flags = AI_PASSIVE; // use my IP

  //Translate address
	if ((rec_rv = getaddrinfo(NULL, MYPORT, &rec_hints, &rec_servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rec_rv));
        return 1;
	}



    //bind socket
    for(rec_p = rec_servinfo; rec_p != NULL; rec_p = rec_p->ai_next) {
        if ((rec_sock = socket(rec_p->ai_family, rec_p->ai_socktype,
                rec_p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(rec_sock, rec_p->ai_addr, rec_p->ai_addrlen) == -1) {
            close(rec_sock);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (rec_p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(rec_servinfo);

    printf("done setting up receiver\n");

    //-------------------------------------------------------------------------//

    //---------------  setup sending socket  ----------------------------------//

    int i;

   	char * node_address[NODE_COUNT];

   	node_address[0] = "192.168.0.101";
   	node_address[1] = "192.168.0.102";
   	node_address[2] = "192.168.0.102";
   	node_address[3] = "192.168.0.103";
   	node_address[4] = "192.168.0.104";
   	node_address[5] = "192.168.0.105";
   	node_address[6] = "192.168.0.106";
   	node_address[7] = "192.168.0.107";


    for(i = 0; i < NODE_COUNT;i++)
    {
	   	memset(&send_hints[i], 0, sizeof send_hints[i]);
	    send_hints[i].ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	    send_hints[i].ai_socktype = SOCK_DGRAM;
	    send_hints[i].ai_flags = AI_PASSIVE; // use my IP

	    // char * node_address;
	    // node_address = (char*) malloc (14); //the size of an IP address is 14 bytes
	 //    char node_address_1[14]; 
		// setAddressWithNodeNumber(i, node_address_1);

		printf("NODE IPADDRESS SET TO: %s\n",node_address[i]);

	    if ((send_rv[i] = getaddrinfo(node_address[i], SERVERPORT, &send_hints[i], &send_servinfo[i])) != 0) {
	        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(send_rv[i]));
	        return 1;
	    }
	    // loop through all the results and make a socket
	    for(send_p[i] = send_servinfo[i]; send_p[i] != NULL; send_p[i] = send_p[i]->ai_next) {
	        if ((send_sock[i] = socket(send_p[i]->ai_family, send_p[i]->ai_socktype,
	                send_p[i]->ai_protocol)) == -1) {
	            perror("talker: socket");
	            continue;
	        }

	        break;
		}

	    if (send_p[i] == NULL) {
	        fprintf(stderr, "talker: failed to bind socket\n");
	        return 2;
	    }

	    freeaddrinfo(send_servinfo[i]);
	}
    printf("done setting up %i senders\n", NODE_COUNT);

    //-------------------------------------------------------------------------//


    

	//-------------------------------------------------------------------------//    

    printf("listener: waiting to recvfrom...\n");

    addr_len = sizeof their_addr;
    
    while(1==1)
    {
       if ((numbytes = recvfrom(rec_sock, buf, MAXBUFLEN-1 , 0,
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


	    //sending the query to a node:
	    char msg[] = "insanely long message that contains a lot of words! IT is even so big that it does not fit in the buffer!!! oeoene oneo neaon aoen oaneo aneo naoen oaen oane oane onaeo";

	    int currentNode = 1;

	    if ((numbytes = sendto(send_sock[currentNode], msg, strlen(msg), 0,
	             send_p[currentNode]->ai_addr, send_p[currentNode]->ai_addrlen)) == -1) {
	        perror("talker: sendto");
	        exit(1);
	    }

    

	    printf("talker: sent %d bytes to %s\n", numbytes, "192.168.0.102");


   }
    close(rec_sock);


    for(i = 0; i < NODE_COUNT;i++)
    {
    	close(send_sock[i]);	
    }
    

    return 0;
}
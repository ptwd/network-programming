#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <pthread.h>

#define MAXPENDING 5    /* Maximum outstanding connection requests */

void DieWithError(char *errorMessage);  /* Error handling function */
void HandleTCPClient(int clntSocket);   /* TCP client handling function */
int AcceptTCPConnection(int servSock);  /* Accept for clients */
void *ThreadMain(void *threadArgs);	

struct ThreadArgs
{
    int clntSock;
};

int main(int argc, char *argv[])
{
    int servSock;                    /* Socket descriptor for server */
    struct sockaddr_in echoServAddr; /* Local address */
    unsigned short echoServPort;     /* Server port */
    unsigned int clntLen;            /* Length of client address data structure */
    unsigned int childProcCount = 0; /* Number of child processes */
    if (argc != 2)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  /* First arg:  local port */

    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
      
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");

    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed");

    for (;;) /* Run forever */
    {
	/* New connection creates a client socket */
	int clntSock = AcceptTCPConnection(servSock);
	printf("port : %d\n", echoServPort);
	
	/* Create separate memory for client argument */
	struct ThreadArgs *threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs));
	if(threadArgs == NULL)
	    DieWithError("malloc() failed");
	threadArgs->clntSock = clntSock;

	/* Create client thread */
	pthread_t threadID;
	if (pthread_create(&threadID, NULL, ThreadMain, (void *)threadArgs) != 0)   /*send clntsock information through threadArgs */
	    DieWithError("pthread_create() failed");
	printf("with thread %ld\n", (long int) threadID);
    }
    /* NOT REACHED */	
	
}

int AcceptTCPConnection(int servSock)
{
    int clntSock;
    struct sockaddr_in echoClntAddr;
    unsigned int clntLen;
    
    /* Set the size of the in-out parameter */
    clntLen = sizeof(echoClntAddr);
    
    /* Wait for a client to connect */
    if((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr,&clntLen)) <0)
	DieWithError("accept() failed");
    
    /* clntSock is connected to a client! */
    printf("client ip : %s\n", inet_ntoa(echoClntAddr.sin_addr));
    return clntSock;
}

void *ThreadMain(void *threadArgs)
{
    /* Guarantees that thread resources are deallocated upon return */
    pthread_detach(pthread_self());

    /* Extract socket file descriptor from argument */
    int clntSock = ((struct ThreadArgs *) threadArgs)->clntSock;
    free(threadArgs);
    /* Deallocate memory for argument */

    HandleTCPClient(clntSock);
    
    return (NULL);
}

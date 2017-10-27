#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define MAXPENDING 5    /* Maximum outstanding connection requests */

void DieWithError(char *errorMessage);  /* Error handling function */
void HandleTCPClient(int clntSocket);   /* TCP client handling function */
int AcceptTCPConnection(int servSock);  /* Accept for clients */
int main(int argc, char *argv[])
{
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
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
	
	/* Fork child process and report any errors */
	pid_t processID = fork();

	if(processID <0)
	    DieWithError("fork() failed");
	else if(processID == 0)		/* If this is the child process */
	{
	    close(servSock);       
	    HandleTCPClient(clntSock);
	    exit(0);
	}
	
	/* parent process */
	printf("with child process : %d\n", (int) processID);
	close(clntSock);		/* Parent closes child scoket descriptor */
	childProcCount++;		/* Increment number of outstanding child processes */
	
	while (childProcCount)		/* Clean up all zombies */
	{
	    processID = waitpid((pid_t) -1, NULL, WNOHANG);    /* same with wait(), but not blocked */
	    if( processID <0)
		DieWithError("waitpid() failed");
	    else if( processID == 0)				/* child is not done -> can accept other clients */
		break;
	    else
	    {
		childProcCount--;
	    }
		    
	}
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

	
    

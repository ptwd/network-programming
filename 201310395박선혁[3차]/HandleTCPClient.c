#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h> /* for close() */
#define RCVBUFSIZE 32 /* Size of receive buffer */
void DieWithError(char *errorMessage); /* Error handling function */
void printFunc(int flag, char *echoBuffer, int messageSize);
void HandleTCPClient(int clntSocket)
{
    char firstResponse[10] = "hi";
    char echoBuffer[RCVBUFSIZE];        /* Buffer for echo string */
    int recvMsgSize;                    /* Size of received message */

    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");
    printFunc(2,echoBuffer,recvMsgSize);

    if (send(clntSocket, firstResponse , recvMsgSize, 0) != recvMsgSize)
	DieWithError("send() failed");
    printf("msg-> %s\n",firstResponse);
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");
    printFunc(2,echoBuffer,recvMsgSize);


    /* Send received string and receive again until end of transmission */
    while (recvMsgSize > 0)      /* zero indicates end of transmission */
    {
        /* Echo message back to client */
        if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
            DieWithError("send() failed");
	printFunc(1,echoBuffer,recvMsgSize);

        /* See if there is more data to receive */
        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");
	printFunc(2,echoBuffer, recvMsgSize);
    }
    close(clntSocket);    /* Close client socket */
}
void printFunc(int flag,char *a, int b)
{
    int i = 0;
    if(flag == 1)
	printf("msg-> ");
    else
	printf("msg<- ");
    for(i= 0; i < b; i++)
       printf("%c",a[i]);
    printf("\n");
}


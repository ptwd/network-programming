#include <stdio.h>      	 /* for printf() and fprintf() */
#include <sys/socket.h> 	 /* for recv() and send() */
#include <unistd.h>     	 /* for close() */
#include <dirent.h>	         /* for file */
#include <string.h>
#define RCVBUFSIZE 32	         /* Size of receive buffer */
#define BUFSIZE 1024

/* Message Type */
#define EchoReq	    01
#define FileUpReq   02
#define EchoRep	    11
#define FileAck	    12
#define Rls	    21
#define Put	    31


void DieWithError(char *errorMessage); /* Error handling function */
void printFunc(int flag, char *echoBuffer, int messageSize);


void HandleTCPClient(int clntSocket)
{
    char MsgType;
    char firstResponse[10] = "hi";

    char echoBuffer[RCVBUFSIZE];        /* Buffer for echo string */
    int recvMsgSize;                    /* Size of received message */
    unsigned int MsgLen;

    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0) /*rcv hello*/
        DieWithError("recv() failed"); 
    printFunc(2,echoBuffer,recvMsgSize);                                 

    if (send(clntSocket, firstResponse ,3, 0) != 3) /* send hi */
        DieWithError("send() failed");
    printf("msg-> %s\n",firstResponse);


    while(1)
    {
	/* Receive message from client */
	if ((recvMsgSize = recv(clntSocket, &MsgType, 1, 0)) < 0)	/* recieve msgtype from clnt */
	    DieWithError("recv() failed");

	if (MsgType == EchoReq)
	{
            if ((recv(clntSocket, &MsgLen, 4, 0)) < 0)       /* save msglen*/
                DieWithError("recv() failed");

	    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
		DieWithError("recv() failed");
	    printFunc(2, echoBuffer, recvMsgSize); 
	    MsgType = EchoRep;    
            if (send(clntSocket, &MsgType, 1, 0) != 1)       /* send EchoRep */
                DieWithError("send() failed");

	    MsgLen = (MsgLen + RCVBUFSIZE-1) /RCVBUFSIZE;    /* loop time */

	    /* Send received string and receive again until end of transmission */
	    while (MsgLen >0)      /* zero indicates end of transmission */
	    {
		/* Echo message back to client */
		if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
		    DieWithError("send() failed");
		printFunc(1,echoBuffer,recvMsgSize);
		MsgLen--;
	    }
	}
	else if(MsgType == FileUpReq) 
	{
	    FILE *fp = NULL;
	    char fileName[256];
	    char fileBuffer[BUFSIZE];
	    int origFileSize;
	    int recvFileSize = 0;
	    
	    if (recv(clntSocket,&origFileSize,32, 0) < 0)
		DieWithError("recv() failed"); 
	    MsgType = FileAck;
            if (send(clntSocket, &MsgType, 1, 0) != 1)       /* send ack */
                DieWithError("send() failed");
            if (recv(clntSocket,fileName,256, 0) < 0)
                DieWithError("recv() failed");
            if((fp = fopen(fileName,"w")) == NULL)
	        DieWithError ("Can not open file");
	    while (origFileSize > recvFileSize)
	    {
	        if ((recvMsgSize = recv(clntSocket,fileBuffer,BUFSIZE, 0)) < 0)
	            DieWithError("recv() failed");
		fwrite(fileBuffer, sizeof(char), recvMsgSize, fp);
		recvFileSize += recvMsgSize;
	    }
	    fclose(fp);
	}
	else if(MsgType == Rls)
	{
            DIR *dir_info;
	    struct dirent *dir_entry;
	    unsigned int dirlen;
	    dir_info = opendir(".");
	    if( NULL != dir_info)
	    {
		while( dir_entry = readdir( dir_info ))
		{
		    dirlen = strlen(dir_entry->d_name);
		    if(send(clntSocket, &dirlen, 4, 0) != 4)
			DieWithError("send() failed in dirlen");
		    if(send(clntSocket, dir_entry->d_name,dirlen,0) != dirlen)
			DieWithError("send() failed"); 
		}
		dirlen = 0;
		if(send(clntSocket, &dirlen, 4, 0) != 4)
                        DieWithError("send() failed in dirlen"); /*send 0 */
		closedir(dir_info);
	    }
	}
	else if(MsgType == Put)
	{
	    FILE *fp = NULL;
	    char fileName[256];
	    char fileBuffer[BUFSIZE];
	    size_t size;	   
	    int fileSize;

	    if (recv(clntSocket,fileName,256, 0) < 0)
	        DieWithError("recv() failed");

            if((fp = fopen(fileName,"r")) == NULL)
                DieWithError ("Can not open file\n");

            fseek(fp, 0, SEEK_END);
            fileSize = ftell(fp);
            fseek(fp, 0, SEEK_SET);                 /* get fileSize */
            if( send(clntSocket,&fileSize,32, 0) != 32)   /* 64Bit int=32 */
                DieWithError("Send() error for fileSize");

	    if( recv(clntSocket, &MsgType, 1, 0) != 1)
		DieWithError("recv() error for fileAck");       /* receive ack from server */
	    if(MsgType != FileAck)
		DieWithError("client  didn't send FileAck");

	    while(!feof(fp))
	    {
		size = fread(fileBuffer, sizeof(char), BUFSIZE, fp);

		if( send(clntSocket,fileBuffer, size, 0)!= size)
		   DieWithError("Send() error for sending file");
	    }
	    fclose(fp);
	}
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


#include <stdio.h>               /* for printf() and fprintf() */
#include <sys/socket.h>          /* for recv() and send() */
#include <unistd.h>              /* for close() */
#define RCVBUFSIZE 32            /* Size of receive buffer */
#define BUFSIZE 1024

/* Message Type */
#define EchoReq     01
#define FileUpReq   02
#define EchoRep     11
#define FileAck     12

void DieWithError(char *errorMessage); /* Error handling function */

void putft(int sock, char servIP[])
{

    FILE *fp = NULL;
    char fileName[256];
    char fileBuffer[BUFSIZE];
    size_t size;
    char MsgType;
    int fileSize;


    printf("filename to put to server -> ");
    scanf("%s",fileName);

    if((fp = fopen(fileName,"r")) == NULL)
        DieWithError ("Can not open file\n");

    printf("Sending => ############\n");
    fflush(stdout);
    MsgType = FileUpReq;

    if( send(sock, &MsgType,1, 0) != 1)    /* notice that client will send file */
        DieWithError("Send() sent a different number of bytes than expected");


    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);                 /* get fileSize */

    if( send(sock,&fileSize,32, 0) != 32)   /* 64Bit int=32 */
        DieWithError("Send() error for fileSize");
    if( recv(sock, &MsgType, 1, 0) != 1)
        DieWithError("recv() error for fileAck");       /* receive ack from server */
    if(MsgType != FileAck)
	DieWithError("server didn't send FileAck");
    if( send(sock,fileName,256, 0) != 256)
        DieWithError("Send() error for fileName");

    while(!feof(fp))
    {
        size = fread(fileBuffer, sizeof(char), BUFSIZE, fp);
	
        if( send(sock,fileBuffer, size, 0)!= size)
            DieWithError("Send() error for sending file");
    }
    fclose(fp);
    printf("%s(%d bytes) uploading success to %s\n\n\n",fileName,fileSize,servIP);
}
            

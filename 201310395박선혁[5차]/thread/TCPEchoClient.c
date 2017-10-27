#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <dirent.h>     /* for file */


#define RCVBUFSIZE 32   /* Size of receive buffer */
#define BUFSIZE 1024    /* */

/* Message Type */
#define EchoReq     01
#define FileUpReq   02
#define EchoRep     11
#define FileAck     12
#define Rls	    21
#define Put	    31


void DieWithError(char *errorMessage);  /* Error handling function */
void putft(int sock, char servIP[]);              /* put in ft */
void lsft();
void rlsft(int sock, char servIP[]);
void getft(int sock, char servIP[]);



int main(void)
{
    int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort;     /* Echo server port */
    char servIP[20];                 /* Server IP address (dotted quad) */
    char echoString[100];            /* String to send to echo server */
    char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
    unsigned int echoStringLen;      /* Length of string to echo */
    unsigned int echoStringLenLen;   /* Length of Length of string to echo */
    int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv() and total bytes read */
    char messageBuffer[100];	     /* Buffer to deliver message */
    char MsgType;

    printf("Server ip : ");
    scanf("%s",servIP);
    printf("port : ");
    scanf("%d",&echoServPort);
    strcpy(echoString,"hello");


    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");


    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family      = AF_INET;             /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    echoServAddr.sin_port        = htons(echoServPort); /* Server port */

  
    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("connect() failed");


    echoStringLen = strlen(echoString);                 /* Determine input length */

    
    /* Send the string to the server */
    if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
	DieWithError("send() sent a different number of bytes than expected");

    
    printf("msg-> %s\n",echoString);                    /* send Hello to server */


    if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        DieWithError("recv() failed or connection closed prematurely");
    echoBuffer[bytesRcvd] = '\0';		        /* Terminate the string! */
    printf("msg<- %s\n",echoBuffer);		        /* Print the echo buffer */
    /* will reveive hi from sever */
    /* end of greetings */

    /* start echoing */
    while(1)
    {
        printf("msg-> ");
        scanf("%s",messageBuffer);
	if(strcmp(messageBuffer,"/quit")==0)
	    break;
	if(strcmp(messageBuffer,"FT")==0)	    /* go to file transfer mode */
        {
	    char com;
	    while(1)
	    {
		printf("Welcome to Socket FT client!\n");
		printf("ftp command [p)ut    g)et    l)s    r)ls    e)xit ] -> ");
		scanf("%s",&com);
		fflush(stdin);
		if(com == 'e')
		    break;
		switch(com)
		{
		    case 'p':
			putft(sock, servIP);
			break;
		    case 'g':
			getft(sock, servIP);
			break;
		    case 'l':
			lsft();			    
			break;
		    case 'r':
			rlsft(sock, servIP);
			break;
		    default:
			break;		
		}
	    }
	    continue;
	}

	fflush(stdin); 
        MsgType = EchoReq;
	if( send(sock, &MsgType,1, 0) != 1)    /* notice that client will send strng */
	     DieWithError("Send() sent a different number of bytes than expected");

        strcpy(echoString,messageBuffer);
        echoStringLen = strlen(echoString);         /* Determine input length */
        /* Send the string to the server */
        
	/*send Length of string */
	if( send(sock, &echoStringLen, 4, 0) != 4)
             DieWithError("send() sent a different number of bytes than expected");
	
	if( send(sock, echoString, echoStringLen, 0) != echoStringLen)
	     DieWithError("send() sent a different number of bytes than expected");

	if( recv(sock, &MsgType, 1, 0) < 0 )
	    DieWithError("recv() error for EchoRep");       /* receive EchoRep */

	if( MsgType != EchoRep )
	    DieWithError("MsgType error");



	/* Receive the same string back from the server */
	totalBytesRcvd = 0;
	printf("msg<- ");                /* Setup to print the echoed string */
	while (totalBytesRcvd < echoStringLen)
	{
	    /* Receive up to the buffer size (minus 1 to leave space for
 *		  a null terminator) bytes from the sender */
	    if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
		DieWithError("recv() failed or connection closed prematurely");
	    totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */
	    echoBuffer[bytesRcvd] = '\0';  /* Terminate the string! */
	    printf(echoBuffer);		   /* Print the echo buffer */
	}
	printf("\n");
    }

    close(sock);
    exit(0);
}

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


void lsft()
{
    DIR *dir_info;
    struct dirent *dir_entry;

    dir_info = opendir(".");
    if( NULL != dir_info)
    {
	while( dir_entry = readdir( dir_info ))
	    printf("%s\n", dir_entry->d_name);
	printf("\n\n");
	closedir(dir_info);
    }
}

void rlsft(int sock, char servIP[])
{
    char MsgTy;
    char *content;
    int rcvMS;
    rcvMS = 1;
    MsgTy = Rls;
    if( send(sock, &MsgTy,1, 0) != 1)
	DieWithError("Send() sent a different number of bytes than expected");
    while(1)
    {
	if( (recv(sock,&rcvMS,4, 0)) < 0  )
            DieWithError("recv() error in rcvMs");
	if( rcvMS == 0)
	    break;
	content = (char *)calloc(rcvMS,sizeof(char));
        if( (recv(sock,content,rcvMS, 0)) < 0  )
            DieWithError("recv() error in rlsft");
	printf("%s\n", content);
	fflush(stdout);
	free(content);
    }
    printf("\n\n");
}
void getft(int sock, char servIP[])
{
    FILE *fp = NULL;
    char fileName[256];
    char fileBuffer[BUFSIZE];
    int origFileSize;
    int recvFileSize = 0;
    int recvMsgSize = 0;
    char MsgTy;

    MsgTy = Put;
    if( send(sock, &MsgTy,1, 0) != 1)		/* send put */
        DieWithError("Send() sent a different number of bytes than expected");

    printf("filename to get from server -> ");
    scanf("%s",fileName);
    
    if( send(sock,fileName,256, 0) != 256)	/* send filename */
	DieWithError("Send() error for fileName");

    if (recv(sock,&origFileSize,32, 0) < 0)
	DieWithError("recv() failed");

    MsgTy = FileAck;
    if (send(sock, &MsgTy, 1, 0) != 1)       /* send ack */
        DieWithError("send() failed");
    if((fp = fopen(fileName,"w")) == NULL)
        DieWithError ("Can not open file");
    while (origFileSize > recvFileSize)
    {
	if ((recvMsgSize = recv(sock,fileBuffer,BUFSIZE, 0)) < 0)
	    DieWithError("recv() failed");
        fwrite(fileBuffer, sizeof(char), recvMsgSize, fp);
        recvFileSize += recvMsgSize;
    }
    fclose(fp);
    printf("%s(%d bytes) downloading success from %s\n\n\n",fileName,origFileSize,servIP);

}  

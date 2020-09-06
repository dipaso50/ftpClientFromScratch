#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "ftpclient.h"
#include "str.h"
#include "srch.h"

char buffer[256];
char* CMD_LIST = "LIST";
char* CMD_PASV = "PASV\n";
char* CMD_QUIT = "QUIT\n";
char* CMD_RETR = "RETR";

struct hostent *server;


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int portFromServerResp(){

    int init = linearSearch(buffer, strlen(buffer), '(');
    int fin = linearSearch(buffer, strlen(buffer), ')');

#if defined PRINTDEBUG
    printf("init %d, fin %d \n", init, fin);
#endif 

    char *strip = substr(buffer, init + 1, fin);

#if defined PRINTDEBUG
    printf("strip is %s \n", strip);
#endif

    return calculatePasivePort(strip);
}

int calculatePasivePort(char *strip){

     //puerto = primer digito * 256 + segundo dígito
     char *token = strtok(strip, ",");
     int numbers[6];

     int i = 0;

     while(token) {
         numbers[i++] = atoi(token);
         token = strtok(NULL, ",");
     }

     int pasivePort = numbers[4] * 256 + numbers[5];

#if defined PRINTDEBUG
     printf("pasive port %d\n", pasivePort);
#endif

     return pasivePort;
}
void listRemote(int controlfd, char* command){

    writefd(controlfd, CMD_PASV);
    readfd(controlfd);

    int pasivePort = portFromServerResp();

    int pasivefd = openSocket(pasivePort, server);

     writefd(controlfd, command);

     readfd(pasivefd);

     readfd(controlfd);
     readfd(controlfd);

     close(pasivefd);
}

void writefd(int fd, char* buff){

    int n = write(fd,buff,strlen(buff));

    if (n < 0) 
        error("ERROR writing to socket");
}

void readfd(int fd){
	bzero(buffer,256);

    int n = read(fd,buffer,255);

    if(n < 0){
        error("Error reading socket");
    }

    printf("%s\n",buffer);
}

int openSocket(int port, struct hostent *server){

    struct sockaddr_in serv_addr;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) 
        error("ERROR opening socket");

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

    serv_addr.sin_port = htons(port);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    return sockfd;
}

void quit(int controlfd){
    writefd(controlfd, CMD_QUIT);
    readfd(controlfd);
}

void retrFile(int controlfd, char* command){

    writefd(controlfd, CMD_PASV);
    readfd(controlfd);

    int pasivePort = portFromServerResp();

    int pasivefd = openSocket(pasivePort, server);

    writefd(controlfd, command);

    readfd(pasivefd);

    readfd(controlfd);
    readfd(controlfd);

    close(pasivefd);
}


int main(int argc, char *argv[])
{
    int controlfd, portno;

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[2]);

    server = gethostbyname(argv[1]);

    controlfd = openSocket(portno, server);

    readfd(controlfd);

    while(1){		
        printf(">>>");

        bzero(buffer,256);
        fgets(buffer,255,stdin);

        char * cmdcpy = (char*) calloc(sizeof(char) , strlen(buffer));
        strcpy(cmdcpy, buffer);

        if(startwith(buffer, CMD_LIST)){
            listRemote(controlfd, cmdcpy);
        }else if(startwith(buffer, CMD_RETR)){
            retrFile(controlfd, cmdcpy);
        }else if(strcmp(CMD_QUIT, buffer) == 0){
            quit(controlfd);
            break;
        }else{
            writefd(controlfd, cmdcpy);
            readfd(controlfd);
        }

        free(cmdcpy);

    }

    printf("Closing socket\n");

    close(controlfd);

    return 0;
}

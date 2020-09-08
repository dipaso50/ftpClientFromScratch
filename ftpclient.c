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

#define BUFFER_LEN 255

char* CMD_LIST = "LIST"; //list directory
char* CMD_PASV = "PASV\n"; //enter in pasive mode
char* CMD_QUIT = "QUIT\n";
char* CMD_RETR = "RETR"; //retrieve file 
char* CMD_CWD = "CWD"; //change working directory
char* CMD_HELP = "HELP"; //change working directory
char* FILE_NOT_FOUND = "550";

struct hostent *server;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int portFromServerResp(char *buff){

    int init = linearSearch(buff, strlen(buff), '(');
    int fin = linearSearch(buff, strlen(buff), ')');

#if defined PRINTDEBUG
    printf("init %d, fin %d \n", init, fin);
#endif 

    char *strip = substr(buff, init + 1, fin);

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

    char *buff = (char*) calloc(BUFFER_LEN, sizeof(char));

    writefd(controlfd, CMD_PASV);

    readfd(controlfd, buff, BUFFER_LEN);

#if defined PRINTDEBUG
    printf("%s\n",buff);
#endif

    int pasivePort = portFromServerResp(buff);

    int pasivefd = openSocket(pasivePort, server);

    writefd(controlfd, command);

    readfdPrint(pasivefd);

    readfdPrint(controlfd);

    writefd(controlfd, "\n");
    readfdPrint(controlfd);

    free(buff);
    close(pasivefd);
}

void writefd(int fd, char* buff){

    int n = write(fd,buff,strlen(buff));

    if (n < 0) 
        error("ERROR writing to socket");
}

void readfdPrint(int fd){

    char *buff = (char*) calloc(BUFFER_LEN, sizeof(char));
    
    readfd(fd, buff, BUFFER_LEN);

    printf("%s\n",buff);

    free(buff);
}

int readfd(int fd, char *buf, int len){

    int n = read(fd,buf,len);

    if(n < 0)
        error("Error reading socket");

    return n;
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
    int buflen = 255;
    char *buff = (char*) calloc(buflen, sizeof(char));

    writefd(controlfd, CMD_QUIT);
    readfd(controlfd, buff, buflen);     
    printf("%s\n",buff);

    free(buff);
}

char* cleanFilename(char * originfn){

    int k = linearSearch(originfn, strlen(originfn), '\n');

    if(k == -1){
        return originfn;
    }

    return substr(originfn, 0, k);
}

int writeInFile(char *filename, char* data){

#if defined PRINTDEBUG
    printf("Filename before (%s)\n", filename);
#endif

    filename = cleanFilename(filename);

#if defined PRINTDEBUG
    printf("Filename after (%s)\n", filename);
#endif

    FILE *fp = fopen(filename,  "w");
     
    if(fp == NULL) {
       printf("file couldn't be opened\n");
       exit(1);
    } 

    int count = fwrite(data,1,strlen(data),fp);
     
    printf("%d bytes written into file %s\n ",count, filename);
         
    fclose(fp); 

    return count;
}

RetrFile* getFileParams(char * command, RetrFile * ret){
     char *token = strtok(command, " ");
     char * cmd;

     int i = 0;

     while(token) {
         if(i == 1){
            ret->remoteFileName = (char*) calloc(strlen(token), sizeof(char));
            strcpy(ret->remoteFileName, token);
         }else if(i == 2){
            ret->localFileName = (char*) calloc(strlen(token), sizeof(char));
            strcpy(ret->localFileName, token);
         }else if(i == 0){
            cmd =  (char*) calloc(strlen(token), sizeof(char));
            strcpy(cmd, token);
         }

         i++;
         token = strtok(NULL, " ");
     }

     int total = strlen(ret->remoteFileName) + strlen(cmd) + 2;

     ret->command = (char*) calloc(total, sizeof(char));

     strcat(ret->command, cmd);
     strcat(ret->command, " ");
     strcat(ret->command, ret->remoteFileName);
     strcat(ret->command, "\n");

#if defined PRINTDEBUG
     printf("Params localFileName (%s), remoteFileName (%s), command (%s)",
             ret->localFileName, ret->remoteFileName ,
             ret->command);
#endif

     free(cmd);

    return ret;    
}

void retrFile(int controlfd, char* command){

    char *buff = (char*) calloc(BUFFER_LEN, sizeof(char));

    writefd(controlfd, CMD_PASV);

    readfd(controlfd, buff,BUFFER_LEN );

#if defined PRINTDEBUG
    printf("%s\n",buff);
#endif

    int pasivePort = portFromServerResp(buff);

    int pasivefd = openSocket(pasivePort, server);

    RetrFile *retFile = (RetrFile *)malloc(sizeof(RetrFile));

    retFile->command = command;

    getFileParams(command, retFile);

    writefd(controlfd, retFile->command);

    //read response to RETR
    bzero(buff, BUFFER_LEN);
    readfd(controlfd, buff, BUFFER_LEN);

    if(startwith(buff, FILE_NOT_FOUND)){
        //RETR file not found
        printf("File %s not found in remote server.\n", 
                 cleanFilename(retFile->remoteFileName));
        return;
    }

    bzero(buff, BUFFER_LEN);

    readfd(pasivefd, buff, BUFFER_LEN);

    writeInFile(retFile->localFileName != NULL ? 
                         retFile->localFileName :
                         retFile->remoteFileName , buff);

    free(retFile);


    writefd(controlfd, "\n");
    readfdPrint(controlfd);

    free(buff);
    close(pasivefd);    
}

void printHelp(){
    printf("*********************************************************************\n");
    printf("LIST [remote directory] - List directory, default current.\n");

    printf("RETR remoteFileName [localFileName] - Get remote file and store");
    printf("in current local directory, for rename file define localFileName.\n");

    printf("CWD directoryName - Change working directory.\n");

    printf("CDUP - Change to parent directory.\n");

    printf("QUIT - Close connection and exit.\n");

    printf("HELP - Print available commands information.\n");
    printf("*********************************************************************\n");
}

int main(int argc, char *argv[])
{
    int controlfd, portno;
    char buffer[BUFFER_LEN];

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[2]);

    server = gethostbyname(argv[1]);

    controlfd = openSocket(portno, server);

    bzero(buffer,BUFFER_LEN);

    readfd(controlfd, buffer, BUFFER_LEN);

    printf("%s\n",buffer);

    while(1){		
        printf(">>>");

        bzero(buffer,BUFFER_LEN);
        fgets(buffer,255,stdin);

        char * cmdcpy = (char*) calloc(sizeof(char) , strlen(buffer));
        strcpy(cmdcpy, buffer);

#if defined PRINTDEBUG
        printf("Command (%s)\n", cmdcpy);
#endif

        if(startwith(buffer, CMD_LIST)){

            listRemote(controlfd, cmdcpy);

        }else if(startwith(buffer, CMD_RETR)){

            retrFile(controlfd, cmdcpy);

        }else if(startwith(buffer, CMD_HELP)){

            printHelp();

        }else if(strcmp(CMD_QUIT, buffer) == 0){
            quit(controlfd);
            break;
        }else{
            bzero(buffer,BUFFER_LEN);
            writefd(controlfd, cmdcpy);
            readfd(controlfd, buffer, BUFFER_LEN);
            printf("%s\n",buffer);
        }

        free(cmdcpy);

    }

    printf("Closing socket\n");

    close(controlfd);

    return 0;
}

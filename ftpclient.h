
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#if !defined(FTPCLIENT_H)
#define FTPCLIENT_H

typedef struct {
    char * remoteFileName;
    char * localFileName;
    char * command;
}RetrFile;

void writefd(int fd, char*buff);
int readfd(int fd , char *buf, int len);
void readfdPrint(int fd);
int openSocket(int port, struct hostent* server);
int calculatePasivePort(char *strip);
int portFromServerResp(char*buff);
void listRemote(int controlfd, char* command);
void quit(int controlfd);
void retrFile(int controlfd, char* command);
int writeInFile(char* filename, char* data);
RetrFile* getFileParams(char * command, RetrFile * ret);
#endif

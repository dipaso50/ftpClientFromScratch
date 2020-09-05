
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#if !defined(FTPCLIENT_H)
#define FTPCLIENT_H
void writefd(int fd, char*buff);
void readfd(int fd);
int openSocket(int port, struct hostent* server);
int calculatePasivePort(char *strip);
int portFromServerResp();
void listRemote(int controlfd);
#endif

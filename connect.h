#ifndef CONNECT_H
#define CONNECT_H

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

typedef struct {
    char *target;
    char *user;
    char *passwd;
    char *connectionType;
    int port;
} socketConnection;

void connectToServer(const char *target, const char *user, const char *passwd, const char *connectionType, int port, int verbose, int silent);

#endif
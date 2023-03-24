#ifndef UTILS_H
#define UTILS_H

#define BUFFER 1024

typedef struct {
    char *target;
    char *user;
    char *passwd;
    char *connectionType;
    int port;
} socketConnection;

void error(char *msg);

#endif 
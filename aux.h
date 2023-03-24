#include <stdio.h>


typedef struct
{
    char *target;
    char *user;
    char *passwd;
    char *connectionType;
    int port;
} socketConnection;

typedef struct
{
    socketConnection *connection;
    FILE *userFile;
    FILE *passFile;
    int verbose;
    int silent;
} bruteForceArgs;



void *bruteForce(void *arg);

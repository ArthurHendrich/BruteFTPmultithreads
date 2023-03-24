#ifndef BRUTEFORCE_H
#define BRUTEFORCE_H

#include "connect.h"

#define BUFFER 1024

typedef struct {
    socketConnection *connection;
    FILE *userFile;
    FILE *passFile;
    int verbose;
    int silent;
} bruteForceArgs;

void bruteForce(bruteForceArgs *args, int threads);

#endif // BRUTEFORCE_H
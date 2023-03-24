#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "aux.h"


#define BUFFER 1024



void usage()
{
    printf("Usage: brute_force [options] target port\n");
    printf("Options:\n");
    printf("  -u <user>         Single username to try\n");
    printf("  -U <file>         File with list of usernames to try\n");
    printf("  -p <pass>         Single password to try\n");
    printf("  -P <file>         File with list of passwords to try\n");
    printf("  -t <threads>      Number of threads to use (default 1)\n");
    printf("  -ipv4             Use IPv4 (default)\n");
    printf("  -ipv6             Use IPv6\n");
    printf("  -v                Verbose output\n");
    printf("  -h                Show this help message\n");
    exit(1);
}


int main(int argc, char *argv[])
{
    int opt, threads = 1, verbose = 0, ipv6 = 0;
    char *target = NULL, *user = NULL, *pass = NULL;
    FILE *userFile = NULL, *passFile = NULL;

    while ((opt = getopt(argc, argv, "u:U:p:P:t:ipv4ipv6vh")) != -1)
    {
        switch (opt)
        {
        case 'u':
            user = optarg;
            break;
        case 'U':
            userFile = fopen(optarg, "r");
            if (userFile == NULL)
            {
                perror("Failed to open user file");
                exit(1);
            }
            break;
        case 'p':
            pass = optarg;
            break;
        case 'P':
            passFile = fopen(optarg, "r");
            if (passFile == NULL)
            {
                perror("Failed to open password file");
                exit(1);
            }
            break;
        case 't':
            threads = atoi(optarg);
            if (threads < 1)
            {
                fprintf(stderr, "Invalid number of threads\n");
                exit(1);
            }
            break;
        case '4':
            ipv6 = 0;
            break;
        case '6':
            ipv6 = 1;
            break;
        case 'v':
            verbose = 1;
            break;
        case 'h':
        default:
            usage();
        }
    }

    if (optind + 2 != argc)
    {
        usage();
    }

    target = argv[optind];
    int port = atoi(argv[optind + 1]);
    if (port <= 0 || port > 65535)
    {
        fprintf(stderr, "Invalid port number\n");
        exit(1);
    }

    bruteForceArgs *args = malloc(sizeof(bruteForceArgs));
    args->connection = malloc(sizeof(socketConnection));
    args->connection->target = target;
    args->connection->user = user;
    args->connection->passwd = pass;
    args->connection->connectionType = ipv6 ? "-ipv6" : "-ipv4";
    args->connection->port = port;
    args->userFile = userFile;
    args->passFile = passFile;
    args->verbose = verbose;
    args->silent = 0;

    bruteForce(&args);

    if (userFile != NULL)
        fclose(userFile);
    if (passFile != NULL)
        fclose(passFile);


    free(args);

    return 0;
}


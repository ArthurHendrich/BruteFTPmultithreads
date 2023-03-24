#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <regex.h>
#include "connect.h"

#define BUFFER 1024


void connectToServer(const char *target, const char *user, const char *passwd, const char *connectionType, int port, int verbose, int silent)
{
    int sockfd;
    char buffer[BUFFER];

    int connectionResult = 0;

    struct sockaddr_in6 server_addr6;
    struct sockaddr_in server_addr;

    if (strcmp(connectionType, "-ipv4") == 0)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            perror("[-] Error creating socket");
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
            perror("[-] Error creating socket");
            exit(1);
        }

        memset(&server_addr6, '0', sizeof(server_addr6));
        server_addr6.sin6_family = AF_INET6;
        server_addr6.sin6_port = htons(port);
        server_addr6.sin6_addr = in6addr_any;

        if (inet_pton(AF_INET6, target, &server_addr6.sin6_addr) <= 0)
        {
            perror("[-] Error connecting to server");
            exit(1);
        }

        connectionResult = connect(sockfd, (struct sockaddr *)&server_addr6, sizeof(server_addr6));
    }
    else
    {
        perror("[-] Error connecting to server");
        exit(1);
    }

    if (connectionResult < 0)
    {
        perror("[-] Error connecting to server");
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
        perror("[-] Error reading from socket");
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "bruteforce.h"

void *threadedBruteForce(void *arg)
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

                connectToServer(target, user, NULL, connectionType, port, verbose, silent);
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

                connectToServer(target, NULL, passwd, connectionType, port, verbose, silent);
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

            connectToServer(target, NULL, NULL, connectionType, port, verbose, silent);
        }
    }

    return NULL;
}

bruteForceArgs *create_bruteForceArgs(bruteForceArgs *original) {
    bruteForceArgs *new_args = (bruteForceArgs *)malloc(sizeof(bruteForceArgs));
    memcpy(new_args, original, sizeof(bruteForceArgs));

    if (original->userFile != NULL) {
        new_args->userFile = fdopen(dup(fileno(original->userFile)), "r");
    }
    if (original->passFile != NULL) {
        new_args->passFile = fdopen(dup(fileno(original->passFile)), "r");
    }

    return new_args;
}

void bruteForce(bruteForceArgs *args, int threads) {
    pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));
    bruteForceArgs **thread_args = malloc(threads * sizeof(bruteForceArgs *));

    for (int i = 0; i < threads; i++) {
        thread_args[i] = create_bruteForceArgs(args);
        if (pthread_create(&thread_ids[i], NULL, threadedBruteForce, (void *)thread_args[i]) != 0) {
            perror("Failed to create thread");
            exit(1);
        }
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
        fclose(thread_args[i]->userFile);
        fclose(thread_args[i]->passFile);
        free(thread_args[i]);
    }

    free(thread_ids);
    free(thread_args);
}
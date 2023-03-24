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

size_t countLines(FILE *file) {
    size_t count = 0;
    int ch;

    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            count++;
        }
    }

    rewind(file);
    return count;
}

void calculateLinesPerThread(size_t totalLines, int threads, size_t *linesPerThread) {
    size_t baseLines = totalLines / threads;
    size_t extraLines = totalLines % threads;

    for (int i = 0; i < threads; i++) {
        linesPerThread[i] = baseLines + (i < extraLines ? 1 : 0);
    }
}

FILE *createTempFileWithLines(FILE *inputFile, size_t startLine, size_t numLines) {
    char buffer[BUFFER];
    size_t currentLine = 0;

    FILE *tempFile = tmpfile();
    if (tempFile == NULL) {
        perror("Failed to create temporary file");
        exit(1);
    }

    rewind(inputFile);
    while (fgets(buffer, BUFFER, inputFile)) {
        if (currentLine >= startLine && currentLine < startLine + numLines) {
            fputs(buffer, tempFile);
        }

        currentLine++;

        if (currentLine >= startLine + numLines) {
            break;
        }
    }

    rewind(tempFile);
    return tempFile;
}

void bruteForce(bruteForceArgs *args, int threads) {
    pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));
    bruteForceArgs **thread_args = malloc(threads * sizeof(bruteForceArgs *));
    
    size_t userFileLines = args->userFile ? countLines(args->userFile) : 1;
    size_t passFileLines = args->passFile ? countLines(args->passFile) : 1;
    size_t totalLines = userFileLines * passFileLines;
    
    size_t *linesPerThread = malloc(threads * sizeof(size_t));
    calculateLinesPerThread(totalLines, threads, linesPerThread);

    size_t currentLine = 0;
    for (int i = 0; i < threads; i++) {
        thread_args[i] = (bruteForceArgs *)malloc(sizeof(bruteForceArgs));
        memcpy(thread_args[i], args, sizeof(bruteForceArgs));

        if (args->userFile != NULL) {
            thread_args[i]->userFile = createTempFileWithLines(args->userFile, currentLine / passFileLines, linesPerThread[i] / passFileLines);
        }
        if (args->passFile != NULL) {
            thread_args[i]->passFile = createTempFileWithLines(args->passFile, currentLine % passFileLines, linesPerThread[i] % passFileLines);
        }

        if (pthread_create(&thread_ids[i], NULL, threadedBruteForce, (void *)thread_args[i]) != 0) {
            perror("Failed to create thread");
            exit(1);
        }

        currentLine += linesPerThread[i];
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
        if (thread_args[i]->userFile != NULL) {
            fclose(thread_args[i]->userFile);
        }
        if (thread_args[i]->passFile != NULL) {
            fclose(thread_args[i]->passFile);
        }
        free(thread_args[i]);
    }

    free(thread_ids);
    free(thread_args);
    free(linesPerThread);
}
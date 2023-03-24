#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

void error(char *msg) {
    perror(msg);
    exit(1);
}
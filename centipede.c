#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "console.h"
#include "centipede.h"

#define GAME_ROWS 24
#define GAME_COLS 80

/**** DIMENSIONS MUST MATCH the ROWS/COLS */
char *GAME_BOARD[] = {
"                   Score:          Lives:",
"=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-centipiede!=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"",
"",
"",
"",
"",
"",
"", 
"" };

pthread_mutexattr_t attributes;
pthread_mutex_t mutex;

void runCentipede() {
    pthread_mutexattr_init(&attributes);
    pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_NORMAL);
    pthread_mutex_init(&mutex, &attributes);
    if (consoleInit(GAME_ROWS, GAME_COLS, GAME_BOARD));

    consoleRefresh();

}
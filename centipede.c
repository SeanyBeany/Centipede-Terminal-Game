#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
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


pthread_mutex_t board_mutex; // mutex for board

int main(int argc, char**argv) 
{
	//int i; // counter for loop iteration
    pthread_t t1;
    pthread_mutex_init(&board_mutex, NULL);
    if (consoleInit(GAME_ROWS, GAME_COLS, GAME_BOARD));
    pthread_create(&t1, NULL, (void *) &movePlayer, NULL);
    sleep(5);
    pthread_mutex_lock(&board_mutex);
    finalKeypress();
    consoleFinish();
    pthread_mutex_unlock(&board_mutex);
}

void movePlayer() {
    fd_set set; // what to check for our select call
    int CHARACTER_HEIGHT = 1;
    bool gameRunning = true;
    int row = 5;
    int col = 30;
    char* ENEMY_BODY[1][1] = {{"@"}};
    char** tile = ENEMY_BODY[0];

    pthread_mutex_lock(&board_mutex);
    consoleDrawImage(row, col, tile, 1);
    consoleRefresh();
    pthread_mutex_unlock(&board_mutex);

    while(gameRunning){
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        struct timespec timeout = getTimeout(1); /* duration of one tick */
        int ret = select(FD_SETSIZE, &set, NULL, NULL, &timeout);	
        if (ret == 0) {
            printf("ret = %d\n", ret);
            printf(" timeout\n");
        }
        while(true){
            if(gameRunning && ret >= 1) {
                char c = getchar();
                pthread_mutex_lock(&board_mutex);
                consoleClearImage(row,col, CHARACTER_HEIGHT, strlen(tile[0]));
                if (c == MOVE_LEFT) {
                    col-= 1;
                } else if (c == MOVE_RIGHT) {
                    col+= 1;
                } else if (c == MOVE_DOWN) {
                    row+= 1;
                } else if (c == MOVE_UP) {
                    row-= 1;
                }
                else if (c == QUIT) {
                    gameRunning = false;
                }
                consoleDrawImage(row, col, tile, CHARACTER_HEIGHT);
                consoleRefresh();
                pthread_mutex_unlock(&board_mutex);

            }
        }

    }
}
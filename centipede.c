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
pthread_mutex_t end_mutex; // mutex for ending program
pthread_cond_t end_signal_mutex; // mutex for signal to end program
pthread_t t1, t2, t3, t4, t5;
int characterRow;
int characterCol;
int gameOver = 0;

void centipedeMain(int argc, char**argv) 
{
    pthread_mutex_init(&board_mutex, NULL);
    if (consoleInit(GAME_ROWS, GAME_COLS, GAME_BOARD));
    if (pthread_create(&t1, NULL, (void *) &movePlayer, NULL) != 0){
        perror("pthread_create");
    }
    if (pthread_create(&t3, NULL, (void *) &refresh, NULL) != 0){
        perror("pthread_create");
    }
    if (pthread_create(&t4, NULL, (void *) &keyboard, NULL) != 0){
        perror("pthread_create");
    }
    if (pthread_create(&t4, NULL, (void *) &upkeep, NULL) != 0){
        perror("pthread_create");
    }
    
    pthread_cond_wait(&end_signal_mutex, &end_mutex);
    pthread_mutex_lock(&board_mutex);
    finalKeypress();
    consoleFinish();
    pthread_mutex_unlock(&board_mutex);
    if (pthread_join(t1, NULL) != 0) {
        perror("pthread_join");
    }
           
    return 0;
}

void movePlayer() {
    bool gameRunning = true;
    characterRow = BOARD_BOTTOM+1;
    characterCol = BOARD_MIDDLE;
    char* CHARACTER[1][1] = {{"@"}};
    char** characterTile = CHARACTER[0];

    pthread_mutex_lock(&board_mutex);
    consoleDrawImage(characterRow, characterCol, characterTile, 1);
    
    pthread_mutex_unlock(&board_mutex);

    while(gameRunning){
        if(gameRunning) {
            char c = getchar();
            if (c == QUIT) {
                pthread_cond_signal(&end_signal_mutex);
                gameRunning = false;
                pthread_mutex_unlock(&board_mutex);
            }
            else if (c == SPACE) {
                pthread_create(&t2, NULL, (void *) &bullet, NULL);
            }
            else {
                pthread_mutex_lock(&board_mutex);
                consoleClearImage(characterRow,characterCol, CHARACTER_HEIGHT, strlen(characterTile[0]));
                if (c == MOVE_LEFT) {
                    if (characterCol >= BOARD_LEFT_SIDE){
                        characterCol-= 1;
                    }
                } else if (c == MOVE_RIGHT) {
                    if (characterCol <= BOARD_RIGHT_SIDE){
                        characterCol+= 1;
                    }
                } else if (c == MOVE_DOWN) {
                    if (characterRow <= BOARD_BOTTOM){
                        characterRow+= 1;
                    }
                } else if (c == MOVE_UP) {
                    if (characterRow >= BOARD_TOP){
                        characterRow-= 1;
                    }
                } else if (c == 'e') {
                    pthread_create(&t2, NULL, (void *) &centipedeBullet, NULL);
                }

                consoleDrawImage(characterRow, characterCol, characterTile, CHARACTER_HEIGHT);
                

                pthread_mutex_unlock(&board_mutex);
            }
        }
    }
}

void bullet() {
    bool gameRunning = true;
    char* BULLET[1][1] = {{"|"}};
    char** bulletTile = BULLET[0];
    int bulletHeight = 0;
    int hit = 0;
    int bulletRow = characterRow;
    int bulletCol = characterCol;

    while(!hit){
        sleepTicks(15);
        if(bulletRow-bulletHeight != 2) {
            pthread_mutex_lock(&board_mutex);
            if(bulletHeight >= 1){
                consoleClearImage(bulletRow-bulletHeight, bulletCol, BULLET_HEIGHT, strlen(bulletTile[0]));
            }
            bulletHeight++;
            consoleDrawImage(bulletRow-bulletHeight, bulletCol, bulletTile, BULLET_HEIGHT);
            
            pthread_mutex_unlock(&board_mutex);
        }
        else {
            pthread_mutex_lock(&board_mutex);
            consoleClearImage(bulletRow-bulletHeight, bulletCol, BULLET_HEIGHT, strlen(bulletTile[0]));
            
            pthread_mutex_unlock(&board_mutex);
            hit = 1;
        }
    }
    pthread_exit(&t2);
}

void centipedeBullet() {
    bool gameRunning = true;
    char* BULLET[1][1] = {{"."}};
    char** bulletTile = BULLET[0];
    int bulletHeight = 0;
    int hit = 0;
    int bulletRow = 1;
    int bulletCol = 33;
    
    while(!hit){
        sleepTicks(15);
        if (bulletRow-bulletHeight == characterRow && bulletCol == characterCol) {
            
        }
        else if(bulletRow-bulletHeight != 15) {
            pthread_mutex_lock(&board_mutex);
            if(bulletHeight <= 15 && bulletHeight != 0){
                consoleClearImage(bulletRow-bulletHeight, bulletCol, BULLET_HEIGHT, strlen(bulletTile[0]));
            }
            bulletHeight--;
            consoleDrawImage(bulletRow-bulletHeight, bulletCol, bulletTile, BULLET_HEIGHT);
            
            pthread_mutex_unlock(&board_mutex);
        }
        else {
            pthread_mutex_lock(&board_mutex);
            consoleClearImage(bulletRow-bulletHeight, bulletCol, BULLET_HEIGHT, strlen(bulletTile[0]));
            
            pthread_mutex_unlock(&board_mutex);
            hit = 1;
        }
    }
    pthread_exit(&t2);
}

void keyboard() {
    fd_set set; // what to check for our select call
    int ret;
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    struct timespec timeout = getTimeout(1); /* duration of one tick */
    ret = select(FD_SETSIZE, &set, NULL, NULL, &timeout);	
    if (ret == 0) {
    printf("ret = %d\n", ret);
    printf(" timeout\n");
    }
    pthread_exit(&t4);
}

void refresh() {
    while(true) {
        sleepTicks(1);
        pthread_mutex_lock(&board_mutex);
        consoleRefresh();
        pthread_mutex_unlock(&board_mutex);
    }
}

void upkeep() {
    char* LIVES3[1][1] = {{"3"}};
    char** lives3 = LIVES3[0];
    char* LIVES2[1][1] = {{"2"}};
    char** lives2 = LIVES2[0];
    char* LIVES1[1][1] = {{"1"}};
    char** lives1 = LIVES1[0];
    char* LIVES0[1][1] = {{"0"}};
    char** lives0 = LIVES0[0];
    pthread_mutex_lock(&board_mutex);
    consoleDrawImage(0, 41, lives3, 1);
    pthread_mutex_unlock(&board_mutex);
    sleep(1);
    pthread_mutex_lock(&board_mutex);
    consoleClearImage(0, 41, 1, strlen(lives2[0]));
    consoleDrawImage(0, 41, lives2, 1);
    pthread_mutex_unlock(&board_mutex);
    sleep(1);
    pthread_mutex_lock(&board_mutex);
    consoleClearImage(0, 41, 1, strlen(lives2[0]));
    consoleDrawImage(0, 41, lives1, 1);
    pthread_mutex_unlock(&board_mutex);
    sleep(1);
    pthread_mutex_lock(&board_mutex);
    consoleClearImage(0, 41, 1, strlen(lives2[0]));
    consoleDrawImage(0, 41, lives0, 1);
    pthread_mutex_unlock(&board_mutex);
    sleep(1);
}
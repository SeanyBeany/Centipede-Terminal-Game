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
pthread_mutex_t fire_mutex; // mutex to cap fire rate
pthread_mutex_t character_mutex; // mutex for character
pthread_mutex_t character_position_mutex; // mutex for character position
pthread_cond_t end_signal_mutex; // mutex for signal to end program
pthread_cond_t fire_mutex_signal; // mutex for signal to cap fire rate
pthread_t t1, t2, t3, t4, t5;
int characterRow = BOARD_BOTTOM; // variable for character row position
int characterCol = BOARD_MIDDLE; // variable for character column poisiton
int gameOver = 0;
int hit = 0; // boolean indicating whether the character has been hit or not

void centipedeMain(int argc, char**argv) 
{
    pthread_mutex_init(&board_mutex, NULL);
    pthread_mutex_init(&fire_mutex, NULL);
    pthread_mutex_init(&end_mutex, NULL);
    pthread_mutex_init(&character_mutex, NULL);
    pthread_mutex_init(&character_position_mutex, NULL);
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

    if (pthread_create(&t5, NULL, (void *) &fireRate, NULL) != 0){
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
    char* CHARACTER[1][1] = {{"@"}};
    char** characterTile = CHARACTER[0];

    pthread_mutex_lock(&board_mutex);
    consoleDrawImage(characterRow, characterCol, characterTile, 1);
    
    pthread_mutex_unlock(&board_mutex);

    while(gameRunning){
        char c = getchar();
        pthread_mutex_lock(&character_mutex);
        if (c == QUIT) {
            pthread_cond_signal(&end_signal_mutex);
            gameRunning = false;
            pthread_mutex_unlock(&board_mutex);
            pthread_mutex_unlock(&character_mutex);
        }
        else if (c == SPACE) {
            pthread_create(&t2, NULL, (void *) &bullet, NULL);
            pthread_mutex_unlock(&character_mutex);
        }
        else {
            pthread_mutex_lock(&board_mutex);
            consoleClearImage(characterRow,characterCol, CHARACTER_HEIGHT, strlen(characterTile[0]));
            if (c == MOVE_LEFT) {
                if (characterCol >= BOARD_LEFT_SIDE){
                    pthread_mutex_lock(&character_position_mutex);
                    characterCol-= 1;
                    pthread_mutex_unlock(&character_position_mutex);
                }
            } else if (c == MOVE_RIGHT) {
                if (characterCol <= BOARD_RIGHT_SIDE){
                    pthread_mutex_lock(&character_position_mutex);
                    characterCol+= 1;
                    pthread_mutex_unlock(&character_position_mutex);
                }
            } else if (c == MOVE_DOWN) {
                if (characterRow <= BOARD_BOTTOM){
                    pthread_mutex_lock(&character_position_mutex);
                    characterRow+= 1;
                    pthread_mutex_unlock(&character_position_mutex);
                }
            } else if (c == MOVE_UP) {
                if (characterRow >= BOARD_TOP){
                    pthread_mutex_lock(&character_position_mutex);
                    characterRow-= 1;
                    pthread_mutex_unlock(&character_position_mutex);
                }
            } else if (c == 'e') {
                pthread_create(&t2, NULL, (void *) &centipedeBullet, NULL);
            }

            consoleDrawImage(characterRow, characterCol, characterTile, CHARACTER_HEIGHT);
            

            pthread_mutex_unlock(&board_mutex);
            pthread_mutex_unlock(&character_mutex);
        }
    }
    
}

void bullet() {
    bool gameRunning = true;
    char* BULLET[1][1] = {{"|"}};
    char** bulletTile = BULLET[0];
    int bulletHeight = 0;
    int offScreen = 0;

    pthread_cond_wait(&fire_mutex_signal, &fire_mutex);  
    int bulletRow = characterRow;
    int bulletCol = characterCol;
    while(!hit && !offScreen){
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
            offScreen = 1;
        }
    }
    
    if(hit) {
        pthread_mutex_lock(&board_mutex);
        consoleClearImage(bulletRow-bulletHeight, bulletCol, BULLET_HEIGHT, strlen(bulletTile[0]));
        pthread_mutex_unlock(&board_mutex);
    }

    pthread_exit(&t2);
}

void fireRate() {
    while(true) {
        pthread_cond_signal(&fire_mutex_signal);
        sleepTicks(50);
    }
}

void centipedeBullet() {
    bool gameRunning = true;
    char* BULLET[1][1] = {{"."}};
    char** bulletTile = BULLET[0];
    int bulletHeight = 0;
    int offScreen = 0;
    int bulletRow = 1;
    int bulletCol = 33;

    while(!hit && !offScreen){
        sleepTicks(15);
        if (bulletRow-bulletHeight == characterRow && bulletCol == characterCol) {
            pthread_mutex_lock(&character_mutex);
            hit = 1;
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
            offScreen = 1;
        }
    }
    
    if(hit) {
        pthread_mutex_lock(&board_mutex);
        consoleClearImage(bulletRow-bulletHeight, bulletCol, BULLET_HEIGHT, strlen(bulletTile[0]));
        pthread_mutex_unlock(&board_mutex);
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
    int lives = 3;
    char* CHARACTER[1][1] = {{"@"}};
    char** characterTile = CHARACTER[0];
    char* CHARACTER_DEAD[1][1] = {{"*"}};
    char** deadCharacterTile = CHARACTER_DEAD[0];


    pthread_mutex_lock(&board_mutex);
    consoleDrawImage(0, 41, lives3, 1);
    pthread_mutex_unlock(&board_mutex);

    while(!gameOver) {
        sleepTicks(100);
        if(hit && lives > 0) {
            pthread_mutex_lock(&board_mutex);
            consoleClearImage(0, 41, 1, strlen(lives3[0]));
            if(lives == 3) {
                consoleDrawImage(0, 41, lives2, 1);
            }
            if(lives == 2) {
                consoleDrawImage(0, 41, lives1, 1);
            }
            if(lives == 1) {
                consoleDrawImage(0, 41, lives0, 1);
            }
            consoleDrawImage(characterRow, characterCol, lives3, 1);
            consoleRefresh();
            sleep(1);
            consoleDrawImage(characterRow, characterCol, lives2, 1);
            consoleRefresh();
            sleep(1);
            consoleDrawImage(characterRow, characterCol, lives1, 1);
            consoleRefresh();
            sleep(1);
            consoleClearImage(characterRow, characterCol, 1, strlen(lives3[0]));
            consoleRefresh();
            
            pthread_mutex_lock(&character_position_mutex);
            characterRow = BOARD_BOTTOM;
            characterCol = BOARD_MIDDLE;
            pthread_mutex_unlock(&character_position_mutex);
            consoleDrawImage(characterRow, characterCol, characterTile, 1);
            pthread_mutex_unlock(&board_mutex);
            hit = 0;
            pthread_mutex_unlock(&character_mutex);
            lives--;
        }

        if(lives == 0) {
            pthread_cond_signal(&end_signal_mutex);
            gameOver = 1;
        }
    }
    
}
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

char* ENEMY_BODY[ENEMY_BODY_ANIM_TILES][ENEMY_HEIGHT] = 
{
  {"1",
   "1"},
  {"2",
   "2"},
  {"3",
   "3"},
  {"4",
   "4"},
  {"5",
   "5"},
  {"6",
   "6"},
  {"7",
   "7"},
  {"8",
   "8"}
};

char* QUIT_TEXT[QUIT_BODY][QUIT_HEIGHT] =
{
    {"p"},
    {"r"},
    {"e"},
    {"s"},
    {"s"},
    {" "},
    {"a"},
    {"n"},
    {"y"},
    {" "},
    {"b"},
    {"u"},
    {"t"},
    {"t"},
    {"o"},
    {"n"},
    {" "},
    {"t"},
    {"o"},
    {" "},
    {"e"},
    {"x"},
    {"i"},
    {"t"},
    {" "},
    {"t"},
    {"h"},
    {"e"},
    {" "},
    {"p"},
    {"r"},
    {"o"},
    {"g"},
    {"r"},
    {"a"},
    {"m"}
};

pthread_mutex_t board_mutex; // mutex for board
pthread_mutex_t end_mutex; // mutex for ending program
pthread_mutex_t fire_mutex; // mutex to cap fire rate
pthread_mutex_t character_mutex; // mutex for character
pthread_mutex_t fired_mutex; // mutex to signal a bullet was fired
pthread_mutex_t character_position_mutex; // mutex for character position
pthread_cond_t end_signal_mutex; // mutex for signal to end program
pthread_cond_t fire_mutex_signal; // mutex for signal to cap fire rate
pthread_cond_t fire_signal_mutex; // mutex to signal a bullet can be fired

pthread_t t1, t2, t3, t4, t5, t6, t7, t8, t9;
int characterRow = BOARD_BOTTOM; // variable for character row position
int characterCol = BOARD_MIDDLE; // variable for character column poisiton
int gameOver = 0;
int hit = 0; // boolean indicating whether the character has been hit or not

void centipedeMain(int argc, char**argv) 
{
    pthread_mutex_init(&board_mutex, NULL);
    pthread_mutex_init(&fire_mutex, NULL);
    pthread_mutex_init(&end_mutex, NULL);
    pthread_mutex_init(&fired_mutex, NULL);
    pthread_mutex_init(&character_mutex, NULL);
    pthread_mutex_init(&character_position_mutex, NULL);
    if (consoleInit(GAME_ROWS, GAME_COLS, GAME_BOARD));
    if (pthread_create(&t1, NULL, (void *) &keyboard, NULL) != 0){
        perror("pthread_create");
    }
    if (pthread_create(&t2, NULL, (void *) &refresh, NULL) != 0){
        perror("pthread_create");
    }
    if (pthread_create(&t3, NULL, (void *) &setUpInput, NULL) != 0){
        perror("pthread_create");
    }
    if (pthread_create(&t4, NULL, (void *) &upkeep, NULL) != 0){
        perror("pthread_create");
    }
    if (pthread_create(&t5, NULL, (void *) &fireRate, NULL) != 0){
        perror("pthread_create");
    }
    if (pthread_create(&t6, NULL, (void *) &character, NULL) != 0){
        perror("pthread_create");
    }
    if (pthread_create(&t9, NULL, (void *) &centipedeSpawner, NULL) != 0){
        perror("pthread_create");
    }
    
    pthread_cond_wait(&end_signal_mutex, &end_mutex);
    pthread_join(t1, NULL);
    
    

    finalKeypress();
    pthread_mutex_lock(&board_mutex);
    consoleFinish();
    pthread_mutex_unlock(&board_mutex);
}

void keyboard() {
    char* CHARACTER[1][1] = {{"@"}};
    char** characterTile = CHARACTER[0];

    pthread_mutex_lock(&board_mutex);
    consoleDrawImage(characterRow, characterCol, characterTile, 1);
    pthread_mutex_unlock(&board_mutex);

    while(!gameOver){
        char c = getchar();
        pthread_mutex_lock(&character_mutex);
        if (c == QUIT) {
            gameOver = true;
            for(int i = 0; i<QUIT_BODY; i++) {
                char** tile = QUIT_TEXT[i];
                pthread_mutex_lock(&board_mutex);
                consoleDrawImage(10, 23+i, tile, CHARACTER_HEIGHT);
                pthread_mutex_unlock(&board_mutex);
            }
            pthread_cond_signal(&end_signal_mutex);
            pthread_mutex_unlock(&character_mutex);
        }
        else if (c == SPACE) {
            pthread_create(&t7, NULL, (void *) &bullet, NULL);
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
                pthread_create(&t8, NULL, (void *) &centipedeBullet, NULL);
            }

            consoleDrawImage(characterRow, characterCol, characterTile, CHARACTER_HEIGHT);
            pthread_mutex_unlock(&board_mutex);
            pthread_mutex_unlock(&character_mutex);
        }
    }
}

void bullet() {
    char* BULLET[1][1] = {{"|"}};
    char** bulletTile = BULLET[0];
    int bulletHeight = 0;
    int offScreen = 0;
    pthread_cond_signal(&fire_signal_mutex); // signal fired so that you couldn't wait and be able to shoot x times faster than 50ms intended fire rate
    pthread_cond_wait(&fire_mutex_signal, &fire_mutex); // Wait for confirmation a bullet can fire
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

}

void fireRate() {
    while(!gameOver) {
        pthread_cond_wait(&fire_signal_mutex, &fired_mutex); // Wait for confirmation of possible bullet fire
        pthread_cond_signal(&fire_mutex_signal); // Signal bullet can be fired
        sleepTicks(50); // Limited fire rate so wait for another bullet to be able to fire
    }
}

void character() {
    char* CHARACTER_ANIMATION_A[1][1] = {{"@"}};
    char** characterTile1 = CHARACTER_ANIMATION_A[0];
    char* CHARACTER_ANIMATION_B[1][1] = {{"$"}};
    char** characterTile2 = CHARACTER_ANIMATION_B[0];

    while(!gameOver) {
        sleepTicks(100);
        pthread_mutex_lock(&character_mutex);
        pthread_mutex_lock(&board_mutex);
        consoleDrawImage(characterRow, characterCol, characterTile1, CHARACTER_HEIGHT);
        pthread_mutex_unlock(&board_mutex);
        pthread_mutex_unlock(&character_mutex);
        sleepTicks(100);
        pthread_mutex_lock(&character_mutex);
        pthread_mutex_lock(&board_mutex);
        consoleDrawImage(characterRow, characterCol, characterTile2, CHARACTER_HEIGHT);
        pthread_mutex_unlock(&board_mutex);
        pthread_mutex_unlock(&character_mutex);
    }
}

void centipedeBullet() {
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
}

void setUpInput() {
    fd_set set; // what to check for our select call
    int ret;
    while(!gameOver) { // set up the keyboard input
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        struct timespec timeout = getTimeout(1); /* duration of one tick */
        ret = select(FD_SETSIZE, &set, NULL, NULL, &timeout);	
        if (ret == -1) {
            printf("Failed to select");
        }
        sleepTicks(20);
        
    }
}

void refresh() {
    while(!gameOver) {
        sleepTicks(1);
        if(pthread_mutex_trylock(&board_mutex) == 0) {
            consoleRefresh();
            pthread_mutex_unlock(&board_mutex);
        }
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
            pthread_mutex_unlock(&board_mutex);
            sleep(1);
            pthread_mutex_lock(&board_mutex);
            consoleDrawImage(characterRow, characterCol, lives2, 1);
            consoleRefresh();
            pthread_mutex_unlock(&board_mutex);
            sleep(1);
            pthread_mutex_lock(&board_mutex);
            consoleDrawImage(characterRow, characterCol, lives1, 1);
            consoleRefresh();
            pthread_mutex_unlock(&board_mutex);
            sleep(1);
            pthread_mutex_lock(&board_mutex);
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

void centipedeSpawner() {
    pthread_t centipede[20];

}
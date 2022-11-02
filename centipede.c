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
    {"-"},
    {"~"},
    {"-"},
    {"~"},
    {"-"},
    {"~"},
    {"-"},
    {"~"}
};

char* ENEMY_BODY2[ENEMY_BODY_ANIM_TILES][ENEMY_HEIGHT] = 
{
  
    {"~"},
    {"-"},
    {"~"},
    {"-"},
    {"~"},
    {"-"},
    {"~"},
    {"-"}
  
};

pthread_mutex_t board_mutex; // mutex for board
pthread_mutex_t end_mutex; // mutex for ending program
pthread_mutex_t fire_mutex; // mutex to cap fire rate
pthread_mutex_t character_mutex; // mutex for character
pthread_mutex_t fired_mutex; // mutex to signal a bullet was fired
pthread_mutex_t character_position_mutex; // mutex for character position
pthread_cond_t end_signal; // mutex for signal to end program
pthread_cond_t fire_mutex_signal; // mutex for signal to cap fire rate
pthread_cond_t fire_signal; // mutex to signal a bullet can be fired
pthread_t t1, t2, t3, t4, t5, t6, t7, t8, t9;
int characterRow = BOARD_BOTTOM; // variable for character row position
int characterCol = BOARD_MIDDLE; // variable for character column poisiton
int gameOver = 0;
int hit = 0; // boolean indicating whether the character has been hit or not
int score = 0; // holds player score

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
    
    pthread_cond_wait(&end_signal, &end_mutex);
    pthread_join(t1, NULL);
    
    

    finalKeypress();
    pthread_mutex_lock(&board_mutex);
    consoleFinish();
    pthread_mutex_unlock(&board_mutex);
    pthread_mutex_destroy(&board_mutex);
    pthread_mutex_destroy(&fire_mutex);
    pthread_mutex_destroy(&end_mutex);
    pthread_mutex_destroy(&fired_mutex);
    pthread_mutex_destroy(&character_mutex);
    pthread_mutex_destroy(&character_position_mutex);
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
                char* s = "Press any button to quit the game";
                int stringLength = 33;
                pthread_mutex_lock(&board_mutex);
                putString(s, MIDDLE_COLUMN, BOARD_MIDDLE-(stringLength/2), stringLength); //Put string "Game Over" in middle of screen
                pthread_mutex_unlock(&board_mutex);
            }
            pthread_cond_signal(&end_signal);
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
    pthread_cond_signal(&fire_signal); // signal fired so that you couldn't wait and be able to shoot x times faster than 50ms intended fire rate
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
        pthread_cond_wait(&fire_signal, &fired_mutex); // Wait for confirmation of possible bullet fire
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

void centipedeBullet(int bulletRow, int bulletCol) {
    char* BULLET[1][1] = {{"."}};
    char** bulletTile = BULLET[0];
    int bulletHeight = 0;
    int offScreen = 0;

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
        char str[5];
        sprintf(str, "%d", score);
        pthread_mutex_lock(&board_mutex);
        putString(str, 0, 25, 5); // put score onto board to the right of Score:
        pthread_mutex_unlock(&board_mutex);
        
        if(score >= 4000) {
            pthread_mutex_lock(&board_mutex);
            pthread_cond_signal(&end_signal);
            char* s = "Congratulations YOU WIN";
            int stringLength = 23;
            putString(s, MIDDLE_COLUMN, BOARD_MIDDLE-(stringLength/2), stringLength); //Put string "Game Over" in middle of screen
            gameOver = 1;
            pthread_mutex_unlock(&board_mutex);
        }

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
            pthread_mutex_lock(&board_mutex);
            pthread_cond_signal(&end_signal);
            char* s = "Game Over";
            int stringLength = 9;
            putString(s, MIDDLE_COLUMN, BOARD_MIDDLE-(stringLength/2), stringLength); //Put string "Game Over" in middle of screen
            gameOver = 1;
            pthread_mutex_unlock(&board_mutex);
        }
    }
}

void centipede(int row, int col) { 
  int j = -7;
  int flip = false;
  char** tile;
  int changeAnimation = false;
  int flipValue = 0;
  int loopsBeforeBullet = 10;
  int pos1[2];
  int tickSpeed = 20; //Speed of ticks between animations
  while(!gameOver){
    pthread_mutex_lock(&character_mutex); //If character gets hit pause the animation
     pthread_mutex_unlock(&character_mutex);
    if(col+j >= 72){
      flip = true;
      pthread_mutex_lock(&board_mutex);
      consoleClearImage(row, j, 1, 8);
      pthread_mutex_unlock(&board_mutex);
      row++;
    }

    if(col+j <= 1 && flip == true) {
      flip = false;
      pthread_mutex_lock(&board_mutex);
      consoleClearImage(row, j, 1, 8);
      pthread_mutex_unlock(&board_mutex);
      row++;
    }

    if(flip){
      j--;
    }
    else {
      j++;
    }

    pthread_mutex_lock(&board_mutex);
    consoleClearImage(row, j+col-1, 1, 1);
    consoleClearImage(row, j+8, 1, 1);
    pthread_mutex_unlock(&board_mutex);
    for (int i = 0; i<ENEMY_BODY_ANIM_TILES; i++) { //loop over the whole enemy animation once 
      if(changeAnimation) {
        if(flip == false) {
          tile = ENEMY_BODY[i];
        }
        else {
          tile = ENEMY_BODY[7-i];
        }
      }
      else {
        if(flip == false) {
          tile = ENEMY_BODY2[i];
        }
        else {
          tile = ENEMY_BODY2[7-i];
        }
      }
      pthread_mutex_lock(&board_mutex);
      if(flip == true) {
          consoleDrawImage(row, col+i+j, tile, ENEMY_HEIGHT);
      }
      else {
          consoleDrawImage(row, col+i+j, tile, ENEMY_HEIGHT);
      }
      pthread_mutex_unlock(&board_mutex);
    }
    sleepTicks(tickSpeed);
    if(loopsBeforeBullet == 0) { 
        if(flip) {
            pos1[0] = row+1;
            pos1[1] = col+j+5;
        }
        else {
            pos1[0] = row+1;
            pos1[1] = col+j+1;
        }
        pthread_create(&t8, NULL, (void *) &bulletLocation, (void*) pos1);
        loopsBeforeBullet = 5;
    }
    else {
        loopsBeforeBullet--;
    }

    if(changeAnimation) {
      changeAnimation = false;
      if(tickSpeed > 10){
        tickSpeed--;
      }
    }
    else {
      changeAnimation = true;
    }
  }
}

void* bulletLocation(void *v)
{
  int* pos = (int*)v;
  centipedeBullet(pos[0], pos[1]);

  return NULL;
}

void* centipedeLocation(void *v)
{
  int* pos = (int*)v;
  centipede(pos[0], pos[1]);

  return NULL;
}

void centipedeSpawner()
{

  pthread_t enemy1, enemy2;
  int pos1[] = {2,0};
  int pos2[] = {2,0};
  pthread_create(&enemy1, NULL, centipedeLocation, (void*)pos1);
  sleep(1000);
  pthread_create(&enemy2, NULL, centipedeLocation, (void*)pos2);

  pthread_join(enemy1, NULL);
  pthread_join(enemy2, NULL);
}

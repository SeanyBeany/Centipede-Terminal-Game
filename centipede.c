/**********************************************************************
  Module: centipede.c
  Author: Shawn Whalen

  Purpose: Runs the centipede console game

**********************************************************************/
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
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

//First body for the caterpillar
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

//Second body for the caterpillar which helps to add a wiggling animation when the body's are swapped
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
pthread_mutex_t character_position_mutex; // mutex for character position
pthread_mutex_t centipede_bullet_mutex; // Mutex for centipede bullet signal
pthread_mutex_t bullet_location; // mutex for bullet location
pthread_cond_t end_signal; // mutex for signal to end program
pthread_cond_t fire_signal; // mutex to signal a bullet can be fired
pthread_cond_t centipede_bullet_signal; // Signal to create centipede
pthread_t t1, t2, t3, t4, t5, t6, t7;
int characterRow = BOARD_BOTTOM; // variable for character row position
int characterCol = BOARD_MIDDLE; // variable for character column poisiton
int gameOver = false; // boolean indicating that the game should end
int hit = false; // boolean indicating whether the character has been hit or not
int score = 0; // holds player score
int bulletPos[2]; // holds the position a bullet should be created at

void centipedeMain(int argc, char**argv) 
{
    /** Initializing the mutex and cond variables
     *  and checking for any creation errors
     */
    if(pthread_mutex_init(&board_mutex, NULL) != 0){ perror("Mutex initialization failed");}
    if(pthread_mutex_init(&fire_mutex, NULL) != 0){ perror("Mutex initialization failed");}
    if(pthread_mutex_init(&end_mutex, NULL) != 0){ perror("Mutex initialization failed");}
    if(pthread_mutex_init(&character_mutex, NULL) != 0){ perror("Mutex initialization failed");}
    if(pthread_mutex_init(&character_position_mutex, NULL) != 0){perror("Mutex initialization failed");}
    if(pthread_mutex_init(&centipede_bullet_mutex, NULL) != 0){perror("Mutex initialization failed");}
    if(pthread_mutex_init(&bullet_location, NULL) != 0){perror("Mutex initialization failed");}
    if(pthread_cond_init(&end_signal, NULL) != 0){perror("Condition initialization failed");}
    if(pthread_cond_init(&fire_signal, NULL) != 0){perror("Condition initialization failed");}
    if(pthread_cond_init(&centipede_bullet_signal, NULL) != 0){perror("Condition initialization failed");}
    
    //error check for initializing the console
    if (consoleInit(GAME_ROWS, GAME_COLS, GAME_BOARD) == false) { printf("Error initializing console");}
    /** Creating the threads for every single main method
     *  and checking for any creation errors
     */
    if (pthread_create(&t1, NULL, (void *) &keyboard, NULL) != 0){ perror("pthread_create");}
    if (pthread_create(&t2, NULL, (void *) &refresh, NULL) != 0){ perror("pthread_create");}
    if (pthread_create(&t3, NULL, (void *) &upkeep, NULL) != 0){perror("pthread_create");}
    if (pthread_create(&t4, NULL, (void *) &character, NULL) != 0){perror("pthread_create");}
    if (pthread_create(&t5, NULL, (void *) &centipedeBulletArrayList, NULL) != 0){perror("pthread_create");}
    if (pthread_create(&t6, NULL, (void *) &centipedeSpawner, NULL) != 0){perror("pthread_create");}
    if (pthread_create(&t7, NULL, (void *) &characterBulletArrayList, NULL) != 0){perror("pthread_create");}
    // We pause until we get a signal to end the game
    if(pthread_cond_wait(&end_signal, &end_mutex) != 0){perror("pthread_cond_wait error");}
    if(pthread_cond_signal(&centipede_bullet_signal) != 0) {perror("Error signaling");} // signal the centipedeBulletArray to finish waiting
    finalKeypress(); // Get the final key press and clear the input buffer
    consoleFinish(); // Terminates the curses cleanly and ends the console
    /** Join all of the threads */
    if(pthread_join(t1, NULL) != 0) {perror("pthread_join error");}
    if(pthread_join(t2, NULL) != 0) {perror("pthread_join error");}
    if(pthread_join(t3, NULL) != 0) {perror("pthread_join error");}
    if(pthread_join(t4, NULL) != 0) {perror("pthread_join error");}
    if(pthread_join(t5, NULL) != 0) {perror("pthread_join error");}
    if(pthread_join(t6, NULL) != 0) {perror("pthread_join error");}
    if(pthread_cond_signal(&fire_signal) != 0){ perror("Error in pthread_cond_signal:");} //send a signal to stop fireRate from waiting indefinitely
    if(pthread_join(t7, NULL) != 0) {perror("pthread_join error");}
    
    /** Destroy all the created mutexes */
    if(pthread_mutex_destroy(&board_mutex) != 0) {perror("Error destroying board mutex");} 
    if(pthread_mutex_destroy(&fire_mutex) != 0) {perror("Error destroying fire mutex");} 
    if(pthread_mutex_destroy(&end_mutex) != 0) {perror("Error destroying end mutex");} 
    if(pthread_mutex_destroy(&character_mutex) != 0) {perror("Error destroying character mutex");} 
    if(pthread_mutex_destroy(&character_position_mutex) != 0) {perror("Error destroying character position mutex");}
    if(pthread_mutex_destroy(&centipede_bullet_mutex) != 0) {perror("Error destroying centipede bullet mutex");}
    if(pthread_mutex_destroy(&bullet_location) != 0) {perror("Error destroying bullet location mutex");} 
    if(pthread_cond_destroy(&end_signal) != 0){perror("Error destroying end signal");}
    if(pthread_cond_destroy(&fire_signal) != 0){perror("Error destroying fire signal");}
    if(pthread_cond_destroy(&centipede_bullet_signal) != 0){perror("Error destroying centipede bullet signal");}
}

/** function that handles user keyboard inputs like character movement (wasd)
 * shooting a bullet (space) and quitting the program (q)
 */
void keyboard() {
    char* CHARACTER[1][1] = {{"@"}}; // character icon
    char** characterTile = CHARACTER[0]; //Setting a pointer to the player icon pointer

    if(pthread_mutex_lock(&board_mutex) != 0) { perror("Error locking:");} 
    consoleDrawImage(characterRow, characterCol, characterTile, 1); //Draw character to the board
    if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");} 

    while(!gameOver){
        fd_set set; // what to check for our select call
        int ret; // return value
        /** setup select to listen to stdin which is necessary as getchar
          *  is blocking and should to be unblocked when ending the game
          * re-set each time as it can get overwritten 
        */
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        struct timeval timeout = getTimeouts(1); // duration of one tick
        ret = select(FD_SETSIZE, &set, NULL, NULL, &timeout);	
        if(ret == -1) {
            perror("Error with select:");
        }
        
        if(!gameOver && ret >= 1){ //Input loop for user input for movement and shooting bullets and quitting the game
            char c = getchar(); //Get character input
            if (pthread_mutex_lock(&character_mutex) != 0){ perror("Error locking character_mutex:");} //lock the character_mutex
            if (c == QUIT) {
                /** if the user presses q it outputs a message to the console and prepares the program to quit */
                gameOver = true; // ends all the loops throughout the program by setting boolean gameOver to true
                for(int i = 0; i<QUIT_BODY; i++) {
                    char* s = "Press any button to quit the game";
                    int stringLength = 33;
                    if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
                    putString(s, MIDDLE_COLUMN, BOARD_MIDDLE-(stringLength/2), stringLength); //Put string "Game Over" in middle of screen
                    if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error locking");}
                }
                if(pthread_cond_signal(&end_signal) != 0){ perror("Error in cond_signal");}
                if(pthread_mutex_unlock(&character_mutex) != 0) {perror("Error unlocking:");}
            }
            else if (c == SPACE) {
                // send a signal to create a bullet thread
                if(pthread_cond_signal(&fire_signal) != 0){ perror("Error in pthread_cond_signal:");}
                if(pthread_mutex_unlock(&character_mutex) != 0) {perror("Error unlocking");}
            }
            else if(c == WIN_CONDITION_TESTING){ // This is for the marker to show that I do have a win condition (press the r button to test it until 4000 score is reached)
                score += 200;
                if(pthread_mutex_unlock(&character_mutex) != 0) {perror("Error unlocking");}
            }
            else {
                /** check user character input and if it is wasd it will move the character in the
                 * corresponding direction locking character_position_mutex as character position
                 * is a critical resource
                 */
                if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
                consoleClearImage(characterRow,characterCol, CHARACTER_HEIGHT, strlen(characterTile[0])); //clear previous character location
                if (c == MOVE_LEFT) {
                    if (characterCol >= BOARD_LEFT_SIDE){
                        if(pthread_mutex_lock(&character_position_mutex) != 0) {perror("Error locking:");}
                        characterCol-= 1;
                        if(pthread_mutex_unlock(&character_position_mutex) != 0) {perror("Error unlocking:");}
                    }
                } else if (c == MOVE_RIGHT) {
                    if (characterCol <= BOARD_RIGHT_SIDE){
                        if(pthread_mutex_lock(&character_position_mutex) != 0) {perror("Error locking:");}
                        characterCol+= 1;
                        if(pthread_mutex_unlock(&character_position_mutex) != 0) {perror("Error unlocking:");}
                    }
                } else if (c == MOVE_DOWN) {
                    if (characterRow <= BOARD_BOTTOM){
                        if(pthread_mutex_lock(&character_position_mutex) != 0) {perror("Error locking:");}
                        characterRow+= 1;
                        if(pthread_mutex_unlock(&character_position_mutex) != 0) {perror("Error unlocking:");}
                    }
                } else if (c == MOVE_UP) {
                    if (characterRow >= BOARD_TOP){
                        if(pthread_mutex_lock(&character_position_mutex) != 0) {perror("Error locking:");}
                        characterRow-= 1;
                        if(pthread_mutex_unlock(&character_position_mutex) != 0) {perror("Error unlocking:");}
                    }
                } 
                consoleDrawImage(characterRow, characterCol, characterTile, CHARACTER_HEIGHT); //Draw character to screen in new location
                if(pthread_mutex_unlock(&board_mutex) != 0) { perror("Error unlocking");}
                if(pthread_mutex_unlock(&character_mutex) != 0) {perror("Error unlocking:");}
            }
        }
    }
}

/** function that draws a bullet that the character shoots that goes upward into the screen
 * until it hits a caterpillar or reaches the top of the screen and acts accordingly
 */
void bullet() {
    char* BULLET[1][1] = {{"|"}}; //bullet icon
    char** bulletTile = BULLET[0]; //setting a pointer to the bullet icon pointer
    int bulletHeight = 0; //the height of the bullet
    int offScreen = 0; //variable to check if bullet if offscreen
    
    //set bullet coordinates to player coordinates
    int bulletRow = characterRow; 
    int bulletCol = characterCol; 
    while(!hit && !offScreen){
        sleepTicks(15);
        if(bulletRow-bulletHeight != 2) { //Check if bullet is going offscreen
            if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
            if(bulletHeight >= 1){ //if bullet is above the player clear bullet icon below
                consoleClearImage(bulletRow-bulletHeight, bulletCol, BULLET_HEIGHT, strlen(bulletTile[0]));
            }
            bulletHeight++; //increment bulletHeight
            consoleDrawImage(bulletRow-bulletHeight, bulletCol, bulletTile, BULLET_HEIGHT); //draw new bullet above old one
            if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
        }
        else {
            /** if the bullet is offScreen clear the image and break the loop so that the thread can be joined */
            if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
            consoleClearImage(bulletRow-bulletHeight, bulletCol, BULLET_HEIGHT, strlen(bulletTile[0]));
            if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
            offScreen = 1;
        }
    }
    
    if(hit) { 
        if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
        consoleClearImage(bulletRow-bulletHeight, bulletCol, BULLET_HEIGHT, strlen(bulletTile[0]));
        if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
    }

}

/** function that handles character animation
 */
void character() {
    char* CHARACTER_ANIMATION_A[1][1] = {{"@"}};
    char** characterTile1 = CHARACTER_ANIMATION_A[0];
    char* CHARACTER_ANIMATION_B[1][1] = {{"$"}};
    char** characterTile2 = CHARACTER_ANIMATION_B[0];

    while(!gameOver) { //Draws the character animation
        sleepTicks(25);
        if(pthread_mutex_lock(&character_mutex) != 0) {perror("Error locking:");}
        if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
        consoleDrawImage(characterRow, characterCol, characterTile1, CHARACTER_HEIGHT);
        if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
        pthread_mutex_unlock(&character_mutex);
        sleepTicks(25);
        pthread_mutex_lock(&character_mutex);
        if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
        consoleDrawImage(characterRow, characterCol, characterTile2, CHARACTER_HEIGHT);
        if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
        pthread_mutex_unlock(&character_mutex);
    }
}

/** function that draws a bullet from the caterpillar that goes downward 
 * until it hits the player or reaches the bottom of the screen and acts accordingly
 */
void centipedeBullet(int bulletRow, int bulletCol) {
    char* BULLET[1][1] = {{"."}}; //centipedeBullet icon
    char** bulletTile = BULLET[0];//pointer to the centipedeBullet icon pointer
    int bulletHeight = 0; //variable that keeps track of bullet height
    int offScreen = 0;

    while(!hit && !offScreen){
        sleepTicks(15);
        //If the character is hit by a bullet break and set hit boolean to true
        if (bulletRow-bulletHeight == characterRow && bulletCol == characterCol) {
            if(pthread_mutex_lock(&character_mutex) != 0) {perror("Error locking:");}
            hit = true;
        }
        //Move the bullet one column above and clear the old bullet tile
        else if(bulletRow-bulletHeight != 15) {
            if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
            if(bulletHeight <= 15 && bulletHeight != 0){
                consoleClearImage(bulletRow-bulletHeight, bulletCol, BULLET_HEIGHT, strlen(bulletTile[0]));
            }
            bulletHeight--;
            consoleDrawImage(bulletRow-bulletHeight, bulletCol, bulletTile, BULLET_HEIGHT);
            
            if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
        }
        else { //bullet is offScreen so we erase the image and break the loop by setting offScreen boolean to true
            if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
            consoleClearImage(bulletRow-bulletHeight, bulletCol, BULLET_HEIGHT, strlen(bulletTile[0]));
            if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
            offScreen = 1;
        }
    }
    
    if(hit) {
        if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
        consoleClearImage(bulletRow-bulletHeight, bulletCol, BULLET_HEIGHT, strlen(bulletTile[0]));
        if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
    }
}

/** method that handles the refresh rate of the console
 */
void refresh() {
    //While game is running every tick we refresh the screen
    while(!gameOver) {
        sleepTicks(1);
        if(pthread_mutex_lock(&board_mutex) == 0) {
            consoleRefresh();
            if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
        }
    }
}

/** A method that handles the upkeep of the program like handling score changes, winning conditions,
 * and character lives.
 */
void upkeep() {
    //These are the 3 characters we use to animate the dead character
    char* LIVES3[1][1] = {{"3"}};
    char** lives3 = LIVES3[0];
    char* LIVES2[1][1] = {{"2"}};
    char** lives2 = LIVES2[0];
    char* LIVES1[1][1] = {{"1"}};
    char** lives1 = LIVES1[0];
    char* LIVES0[1][1] = {{"0"}};
    char** lives0 = LIVES0[0];
    int lives = 3; //Lives starts out at 3
    char* CHARACTER[1][1] = {{"@"}};
    char** characterTile = CHARACTER[0];

    if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
    consoleDrawImage(0, 41, lives3, 1); //Draw the live counter to the board
    if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}

    while(!gameOver) {
        sleepTicks(1);
        char str[5]; //Holds the current score number as a string
        sprintf(str, "%d", score); //gets the current score number and converts it to a string stored in str
        if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
        putString(str, 0, 25, 5); // put score onto board to the right of Score:
        if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
        /** If the score > 4000 we get the winning condition since there are no caterpillars left 
         * (not implemented yet but each caterpillar segments should be 100 points so 8*5 total segments should be 4000 points)
         * where we print "Congratulations YOU WIN"and then we signal the game to end by setting 
         * boolean gameOver to true
         */
        if(score >= 4000) { 
            if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
            pthread_cond_signal(&end_signal);
            char* s = "Congratulations YOU WIN";
            int stringLength = 23;
            putString(s, MIDDLE_COLUMN, BOARD_MIDDLE-(stringLength/2), stringLength); //Put string "Game Over" in middle of screen
            gameOver = true;
            if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
        }

        /** if the character is hit we pause the game and we have a character respawn animation
         *  along with decreasing the lives counter by 1 and spawning the character in the middle of the screen
         */
        if(hit && lives > 0) {
            if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
            consoleClearImage(0, 41, 1, strlen(lives3[0]));
            // decrease the lives counter at the top of the screen by 1
            if(lives == 3) {
                consoleDrawImage(0, 41, lives2, 1);
            }
            if(lives == 2) {
                consoleDrawImage(0, 41, lives1, 1);
            }
            if(lives == 1) {
                consoleDrawImage(0, 41, lives0, 1);
            }
            
            /** draw a countdown animation where the character died and then respawn the character
             *  when the countdown animation finishes
             */
            consoleDrawImage(characterRow, characterCol, lives3, 1);
            consoleRefresh();
            if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
            sleepTicks(20);
            if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
            consoleDrawImage(characterRow, characterCol, lives2, 1);
            consoleRefresh();
            if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
            sleepTicks(20);
            if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
            consoleDrawImage(characterRow, characterCol, lives1, 1);
            consoleRefresh();
            if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
            sleepTicks(20);
            if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
            consoleClearImage(characterRow, characterCol, 1, strlen(lives3[0]));
            consoleRefresh();
            // set the character position to the middle of the board
            pthread_mutex_lock(&character_position_mutex); 
            characterRow = BOARD_BOTTOM; 
            characterCol = BOARD_MIDDLE;
            pthread_mutex_unlock(&character_position_mutex);
            consoleDrawImage(characterRow, characterCol, characterTile, 1); // draw the character in the middle of the board
            if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
            hit = false; // set hit to false
            pthread_mutex_unlock(&character_mutex);
            lives--;
        }

        if(lives == 0) { // if there are no lives left trigger the losing ending where we print "Game Over" and signal the game to end
            if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
            pthread_cond_signal(&end_signal);
            char* s = "Game Over";
            int stringLength = 9;
            putString(s, MIDDLE_COLUMN, BOARD_MIDDLE-(stringLength/2), stringLength); //Put string "Game Over" in middle of screen
            gameOver = true;
            if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
        }
    }
}

/** function that creates a centipede that crawls across the board and shoots bullets
 * that are handled by the centipedeBullet function
 */
void centipede(int row, int col) {
    int size = 8; // size of the caterpillar
    int j = -7; // used to calculate the correct column index to draw and clear centipede segments
    int flip = false; //Variable to determine if the centipede was flipped
    char** tile;
    int changeAnimation = false; //Variable to determine if the animation was changed
    int loopsBeforeBullet = 10; // loops before a centipede bullet is created
    int tickNumber = 20; //number of ticks between animations
    int loopCounter = 0; //Counter for how many loops have elapsed since the last tick speed increase
    int rightWrappingIndex = 80-size+1; // index used for caterpillar wrapping when hitting the right boarder of the game board
    int leftWrappingIndex = size-2; // index used for caterpillar wrapping when hitting the left boarder of the game board
    while(!gameOver){
    pthread_mutex_lock(&character_mutex); //If character gets hit pause the animation
    pthread_mutex_unlock(&character_mutex);
    
    if(row == 11) { //If centipede hits the bottom of the screen delete it and create a new centipede at the top
        if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
        consoleClearImage(row, j, 1, size);
        if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
        row = 2;
        col = 0;
        j = -7;
        flip = false;
        changeAnimation = false;
    }

    if(col+j >= BOARD_RIGHT_BORDER-size){ //If the centipede is at the right side of the screen set boolean flip to true and animate the centipede wrapping downward
        flip = true; //sets boolean true if the centipede flipped (is on an odd row)
        for(int j = 0; j < size; j++) { // wrapping animation for the centipede
            pthread_mutex_lock(&character_mutex); //If character gets hit pause the animation
            pthread_mutex_unlock(&character_mutex);
            for (int i = 0; i<size; i++) { //loop over the whole centipede animation once 
                tile = ENEMY_BODY[i];
                if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
                consoleClearImage(row, rightWrappingIndex-1, 1, 1);  // Clear the previous segment drawing in front of the centipede
                if(rightWrappingIndex+i >= BOARD_RIGHT_BORDER) { // if the centipede is off the screen start drawing it below going in the opposite direction
                    consoleDrawImage(row+1, BOARD_RIGHT_BORDER-size+i, tile, ENEMY_HEIGHT); // Draw the tile to the screen
                }
                consoleDrawImage(row, rightWrappingIndex, tile, ENEMY_HEIGHT); // Draw the tile to the screen
                if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
            }
        sleepTicks(tickNumber); // sleep for tickNumber ticks
        rightWrappingIndex++;
      }
      row++;
      rightWrappingIndex = 80-size+1; // reset the variable to original value
    }

    if(col+j <= 0 && flip == true) { //If the centipede is at the left side of the screen set flip to false and animate the centipede wrapping downward
      flip = false; //sets boolean false if the centipede flipped (is on an even row)
      for(int j = 0; j < size; j++) { // wrapping animation for the centipede
            pthread_mutex_lock(&character_mutex); //If character gets hit pause the animation
            pthread_mutex_unlock(&character_mutex);
            for (int i = 0; i<size; i++) { //loop over the whole centipede animation once 
                tile = ENEMY_BODY[i];
                if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
                consoleClearImage(row, leftWrappingIndex+1, 1, 1);  // Clear the previous segment drawing in front of the centipede
                if(leftWrappingIndex-i < BOARD_LEFT_BORDER) { // if the centipede is off the screen start drawing it below going in the opposite direction
                    consoleDrawImage(row+1, BOARD_LEFT_BORDER+size-i-1, tile, ENEMY_HEIGHT); // Draw the tile to the screen
                }
                consoleDrawImage(row, leftWrappingIndex-1, tile, ENEMY_HEIGHT); // Draw the tile to the screen
                if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
            }
        sleepTicks(tickNumber); // sleep for tickNumber ticks
        leftWrappingIndex--;
      }
      row++;
      leftWrappingIndex = size-2; // reset the variable to original value
    }
    
    if(flip){ // Move the column index variable one left  
      j--;
    }
    else {
      j++; // move the column index variable one right
    }

    if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
    consoleClearImage(row, j+col-1, 1, 1); // Clear the previous segment drawing behind the centipede
    if(flip) {
        consoleClearImage(row, j+size, 1, 1);  // Clear the previous segment drawing in front of the centipede
    }
    if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
    for (int i = 0; i<size; i++) { //loop over the whole centipede animation once 
      if(changeAnimation) { // If the animation is changed we use the first centipede animation
        if(flip == false) { // if the centipede is flipped we draw it reversed, otherwise we draw it regularly
          tile = ENEMY_BODY[i]; 
            }
        else {
          tile = ENEMY_BODY[size-1-i];
            }
        }
        else { // If the animation is not changed we use the second centipede animation
        if(flip == false) { // if the centipede is flipped we set the tiles to be reversed, otherwise it is regular centipede tiles
          tile = ENEMY_BODY2[i];
        }
        else {
            tile = ENEMY_BODY2[size-1-i];
            }
        }
        if(pthread_mutex_lock(&board_mutex) != 0) {perror("Error locking:");}
        consoleDrawImage(row, col+i+j, tile, ENEMY_HEIGHT); // Draw the tile to the screen
        if(pthread_mutex_unlock(&board_mutex) != 0) {perror("Error unlocking:");}
    }
    sleepTicks(tickNumber); // sleep for tickNumber ticks
    if(loopsBeforeBullet == 0) { // if loopsBeforeBullet is 0 we create a new bullet below the centipede
        if(pthread_mutex_lock(&bullet_location) != 0) {perror("Error locking:");}
        if(flip) {
            bulletPos[0] = row+1;
            bulletPos[1] = col+j+4;
        }
        else {
            bulletPos[0] = row+1;
            bulletPos[1] = col+j+1;
        }
        if(pthread_cond_signal(&centipede_bullet_signal) != 0) {perror("Error signaling");}
        if(gameOver) { // avoids centipede deadlocking
            pthread_mutex_unlock(&bullet_location);
        }
        loopsBeforeBullet = 5;
    }
    else {
        loopsBeforeBullet--;
    }

    if(changeAnimation) { // if the animation was changed we change it back
      changeAnimation = false;
      loopCounter++;
      if(tickNumber > 3 && loopCounter == 20){ // we decrement the tickNumber of the centipede which makes it move and shoot bullets faster
        tickNumber--;
        loopCounter = 0;
      }
    }
    else {
      changeAnimation = true;
    }
  }
}

/** function that handles the bullet location for creating a centipede bullet
 */
void* bulletLocation(void *v)
{
  int* pos = (int*)v; // convert to int*
  centipedeBullet(pos[0], pos[1]); //call method that handles the centipedeBullet

  return NULL;
}

/** function that handles the centipede location for creating a centipede
 */
void* centipedeLocation(void *v)
{
  int* pos = (int*)v; // convert to int*
  centipede(pos[0], pos[1]); //call method that handles the centipede

  return NULL;
}

/** function that spawns centipedes
 */
void centipedeSpawner()
{
  pthread_t enemy1[20];
  int r; // variable for random time between spawning centipedes
  int pos1[] = {2,0};
  int threadNumber = 0; // number of threads spawned
  int i; // index for loop
  int maxEnemies = 5; // maximum number of enemies to spawn
  /** spawn centipede and wait for a random time between above 10 and below 24 seconds */
  while(maxEnemies > 0 && !gameOver) {
    pthread_create(&enemy1[threadNumber], NULL, centipedeLocation, (void*)pos1);
    r = (rand() % 10)+10;
    r *= 50; // multiply by 50 since we sleep ticks and 1s = 1000ms and 1000/20 = 50
    for(i = 0; i<r ; i++){
        if(gameOver){ // if the game is over break so that we don't wait 10+ seconds before exiting the thread
            break;
        }
        sleepTicks(1);
        }
    threadNumber++;
    maxEnemies--;
    }

    for(i = 0; i < threadNumber; i++) {
        pthread_join(enemy1[i], NULL);
    }
}



 
#define TIMESLICE_USEC 10000
#define TIME_USECS_SIZE 1000000
#define USEC_TO_NSEC 1000  
/** function that returns a timeval struct with the given tick timeout interval
 */
struct timeval getTimeouts(int ticks) //function for select 
{
  struct timeval rqtp;

  /* work in usecs at first */
  rqtp.tv_usec = TIMESLICE_USEC * ticks; //setting up the number of microseconds

  /* handle usec overflow */
  rqtp.tv_sec = rqtp.tv_usec / TIME_USECS_SIZE;
  rqtp.tv_usec %= TIME_USECS_SIZE;

  return rqtp;
}

void insert_end(Node** head, pthread_t* t) { //Function that inserts a node to the end of the linked list
    Node* node = malloc(sizeof(Node));
    if(node == NULL) {
        return;
    }
    node->next = NULL;
    node->t = t;

    if(*head == NULL) {
        *head = node;
        return;
    }
    
    Node* curr = *head;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = node;
}

void remove_head(Node** head) { //Function that deletes the head node in a linked list and sets the next node as the head
    if (*head == NULL) {
        return;
    }

    Node* temp = *head;
    *head = (*head)->next;
    free(temp); // free the pthread_t pointers memory
}

/** function that manages centipede bullets in an array list
 */
void centipedeBulletArrayList() {
    int currentBulletPos[2];
    Node* head = NULL; // create the head node
    pthread_t* p; // pointer to the first thread in the node
    int i = 0; // counter index for while loop
    int threadCounter = 0; // counter of total threads
    int removeIndex = 0; // index to remove next head and join the thread
    pthread_t th[CENTIPEDE_BULLET_ARRAY_MAX]; // initialization of pthread_t
    if(pthread_mutex_init(&centipede_bullet_mutex, NULL) != 0){perror("Error initializing mutex:");} //initialize mutex
    while(!gameOver) { // while game is running create a new thread when a signal to create a new thread is received
        if(i == CENTIPEDE_BULLET_ARRAY_MAX) { // to avoid overflow if we reach the top of the array we reset to the beginning
            i = 0;
        }
        if(pthread_cond_wait(&centipede_bullet_signal, &centipede_bullet_mutex) != 0){perror("pthread_cond_wait");}
        if(gameOver) {
            break; // need to break if the game is over so we can free threads
        }
        currentBulletPos[0] = bulletPos[0];
        currentBulletPos[1] = bulletPos[1];
        pthread_mutex_unlock(&bullet_location); // unlock bullet_location since bullet location was stored in this program and it's a critical resource
        if(pthread_create(&th[i], NULL, (void *) &bulletLocation, (void*) currentBulletPos) != 0){perror("pthread_create");}
        insert_end(&head, &th[i]);
        threadCounter++;
        i++;
        if(threadCounter-removeIndex > 60) { // joining the old offscreen threads (there can be over 50 bullets on screen at a time so for safety we clear after 60)
            p = head->t;
            remove_head(&head);
            if(pthread_join(*p, NULL) !=0 ) {printf("Error joining thread");}
            removeIndex++;
        }
    }
    
    for(i = removeIndex; i < threadCounter; i++) {
        p = head->t;
        remove_head(&head);
        if(pthread_join(*p, NULL) !=0 ) {printf("Error joining thread");}
    }
}

/** function that manages character bullets in an array list
 */
void characterBulletArrayList() {
    Node* head = NULL; // create the head node
    pthread_t* p; // pointer to the first thread in the node
    int i = 0; // counter index for while loop
    int threadCounter = 0; // counter of total threads
    pthread_t th[CHARACTER_BULLET_ARRAY_MAX]; // initialization of pthread_t
    int removeIndex = 0; // index to remove next head and join the thread
    while(!gameOver) { // while game is running create a new thread when a signal that we can create a new bullet thread is received
    if(i == CHARACTER_BULLET_ARRAY_MAX) { // to avoid overflow if we reach the top of the array we reset to the beginning
        i = 0;
    }
    // Wait for confirmation a bullet can fire
    if(pthread_cond_wait(&fire_signal, &fire_mutex) != 0) { perror("Error in pthread_cond_wait");}
        if(gameOver) {
            break; // need to break if the game is over so we can free threads
        }
        if(pthread_create(&th[i], NULL, (void *) &bullet, NULL) != 0){perror("pthread_create");}
        insert_end(&head, &th[i]);
        threadCounter++;
        i++;
        sleepTicks(60); // limits fire rate
        if(threadCounter-removeIndex > 10) { // joining the old offscreen threads
            p = head->t;
            remove_head(&head);
            if(pthread_join(*p, NULL) !=0 ) {printf("Error joining thread");}
            removeIndex++;
        }
    }
    
    for(i = removeIndex; i < threadCounter; i++) { // joining last left over threads
        p = head->t;
        remove_head(&head);
        if(pthread_join(*p, NULL) !=0 ) {printf("Error joining thread");}
    }
}

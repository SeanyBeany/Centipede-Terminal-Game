/**********************************************************************
  Module: centipede.h
  Author: Shawn Whalen

  Purpose: the .h file for centipede.c which defines all the macros and functions for the centipede.h program

**********************************************************************/
#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <pthread.h>

//will probably want a file just for game globals. There should be a lot!
#define MOVE_LEFT 'a'
#define MOVE_RIGHT 'd'
#define MOVE_UP 'w'
#define MOVE_DOWN 's'
#define SHOOT ' '
#define QUIT 'q'
#define SPACE ' '
#define CHARACTER_HEIGHT 1
#define BULLET_HEIGHT 1
#define PTHREAD_MUTEX_NORMAL 0
#define CENTIPEDE_BULLET_ARRAY_MAX 200
#define CHARACTER_BULLET_ARRAY_MAX 50
#define MAX_CHARACTER_BULLETS_ON_SCREEN 10
#define MAX_CENTIPEDE_BULLETS_ON_SCREEN 120
#define BOARD_TOP 13
#define BOARD_BOTTOM 14
#define BOARD_LEFT_SIDE 1
#define BOARD_RIGHT_SIDE 78
#define BOARD_RIGHT_BORDER 80
#define BOARD_LEFT_BORDER 0
#define BOARD_MIDDLE 40
#define MIDDLE_COLUMN 8
#define ENEMY_HEIGHT 1
#define ENEMY_BODY_ANIM_TILES 8 
#define QUIT_BODY 36
#define QUIT_HEIGHT 1
#define GLOBAL_BULLET_ARRAY_MAX 10

/** linked list structure which contains
 * pointers to a pthread_t
 */
typedef struct Node {
    pthread_t* t;
    struct Node* next;
} Node;

void centipedeMain();
void runCentipede();
void movePlayer();
void bullet();
void centipedeBullet(int bulletRow, int bulletCol);
void refresh();
void keyboard();
void upkeep();
void character();
void centipede(int row, int col, int size);
void* bulletLocation(void *v);
void* centipedeLocation(void *v);
void centipedeSpawner();
void setUpInput();
void insert_end(Node** head, pthread_t* t);
void remove_head(Node** head);
void centipedeBulletArrayList();
void characterBulletArrayList();

struct timeval getTimeouts(int ticks);



#endif

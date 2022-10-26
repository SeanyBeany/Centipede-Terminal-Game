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
#define BOARD_TOP 13
#define BOARD_BOTTOM 14
#define BOARD_LEFT_SIDE 1
#define BOARD_RIGHT_SIDE 78
#define BOARD_MIDDLE 40
#define ENEMY_HEIGHT 2
#define ENEMY_BODY_ANIM_TILES 8 
#define QUIT_BODY 36
#define QUIT_HEIGHT 1

void centipedeMain();
void runCentipede();
void movePlayer();
void bullet();
void centipedeBullet();
void refresh();
void keyboard();
void upkeep();
void fireRate();
void setUpInput();
void character();
void centipedeSpawner();
#endif

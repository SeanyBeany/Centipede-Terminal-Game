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
#define CENTIPEDE_BULLET_ARRAY_MAX 100
#define CHARACTER_BULLET_ARRAY_MAX 50
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
#define WIN_CONDITION_TESTING 'r'
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
void centipede(int row, int col);
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

/**********************************************************************
  Module: main.c
  Author: Shawn Whalen

  Purpose: calls the centipede.c file to run the centipede game and 
  waits for the game to finish before printing "done!" and returning

**********************************************************************/
#include <stdio.h>
#include "centipede.h" 

int main(int argc, char**argv) 
{
	centipedeMain();
	printf("done!\n");
	return 0;
}

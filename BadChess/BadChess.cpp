// BadChess.cpp : Defines the entry point for the application.
//
#include "stdio.h"
#include "stdlib.h"
#include "defs.h"


int main()
{
	AllInit();

	int move = 0;
	int from = A2;
	int to = H7;
	int cap = wR;
	int prom = bQ;
	move = (from) | (to << 7) | (cap << 14) | (prom << 20);

	printf("Algebraic from:%s\n", PrSq(from));
	printf("Algebraic to:%s\n", PrSq(to));
	printf("Algebraic move:%s\n", PrMove(move));

	return 0;
};
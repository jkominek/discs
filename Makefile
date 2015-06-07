discs: discs.c
	gcc --std=c99 -O3 -ffast-math discs.c -o discs -lnlopt -lm

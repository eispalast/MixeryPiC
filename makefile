prog:
	gcc -pthread -Wall -std=c99 main.c network.h LCD.h channel.h -o mix -lwiringPi -lwiringPiDev 


CC=gcc
C_FLAGS=-Wall -O3 -o live-server.bin

build: 
	$(CC) $(C_FLAGS) src/*.c

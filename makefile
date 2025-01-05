
CC=gcc
C_FLAGS=-Wall -O3 -lpthread -o live-server.bin

build: 
	$(CC) $(C_FLAGS) src/*.c

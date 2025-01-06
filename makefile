
CC=gcc
C_FLAGS=-Wall -O3 -lpthread -o live-server.bin -g

build: 
	$(CC) $(C_FLAGS) src/*.c

CC=gcc

all: main.c
	$(CC) -g main.c -o main

run: all
	sudo ./main

val: all
	sudo valgrind -s ./main

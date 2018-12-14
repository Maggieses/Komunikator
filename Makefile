all:
	gcc -pthread -ljansson -lmariadb server.c handler.c -o server

all:
	gcc -Wall -Werror -pthread -ljansson -lmariadb server.c handler.c -o server

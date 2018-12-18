#ifndef HANDLER_H
#define HANDLER_H
#include <mariadb/mysql.h>

MYSQL *conn;

#define HOST "localhost"
#define USER "handler"
#define PASS "1234"
#define DATABASE "Komunikator"
#define PORT 3306
#define SOCKET NULL
#define OPTIONS 0

struct thread_args{
	int sock;
};

void* handler(void *args);

#endif

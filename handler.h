#ifndef HANDLER_H
#define HANDLER_H

struct thread_args{
	int sock;
};

void* handler(void *args);

#endif

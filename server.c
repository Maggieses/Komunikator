#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include "handler.h"
#include <stdlib.h>
#include <stdio.h>

#define PORT_L 20000
#define THREADS 100

int main (void) {
	int gniazdo;
	pthread_t tmp_thread_id;
	struct thread_args* tmp_args;
	struct sockaddr_in adr,nadawca;
	socklen_t dl= sizeof (struct sockaddr_in);

	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,HOST,USER,PASS,DATABASE,PORT,SOCKET,OPTIONS))return 2;

	gniazdo = socket (PF_INET, SOCK_STREAM, 0);
	adr.sin_family = AF_INET;
	adr.sin_port = htons(PORT_L);
	adr.sin_addr.s_addr = INADDR_ANY;
	if (bind(gniazdo, (struct sockaddr*) &adr, sizeof(adr)) < 0){
		return 1;
	}
	if(listen(gniazdo, THREADS) < 0) {
		return 1;
	}
	
	printf("Listens on %d\n",PORT_L);

	while(1){
		tmp_args=malloc(sizeof(*tmp_args));
		tmp_args->sock=accept(gniazdo,(struct sockaddr*) &nadawca,&dl);
		pthread_create(&tmp_thread_id,NULL,handler,(void *)tmp_args);
		pthread_detach(tmp_thread_id);

	}

	return 0;
}

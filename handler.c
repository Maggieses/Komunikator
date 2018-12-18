#include "handler.h"
#include <sys/socket.h>
#include <jansson.h>
#include <string.h>
#include <mariadb/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEBUG


static void handle_recv_messages(int sock,const char *user){
	int i;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char msg[10240];
	char *lbracket="[";
	char *rbracket="]";
	char SELECT[1024];
	snprintf(SELECT,1023,"SELECT source,msg FROM Messages WHERE target='%s';",user);
	if(mysql_real_query(conn,SELECT,strlen(SELECT))){return;}
	res=mysql_store_result(conn);
	if(res==NULL){
		#ifdef DEBUG
		printf("NULL RESOURCES!");
		#endif
		return;
	}
	send(sock,lbracket,strlen(lbracket),0);
	i=0;
#ifdef DEBUG
	printf("Sending works until loop\n");
#endif
	while((row=mysql_fetch_row(res))){
		if(i++)send(sock,",",strlen(","),0);
		#ifdef DEBUG
		printf("%s %s\n",row[0],row[1]);
		#endif
		snprintf(msg,10239,"{\"from\":\"%s\",\"msg\":\"%s\" }",row[0],row[1]);
		#ifdef DEBUG
		printf("\nMessage sent: %s\n\n",msg);
		#endif
		send(sock,msg,strlen(msg),0);
	}
	send(sock,rbracket,strlen(rbracket),0);
	mysql_free_result(res);
}

static void handle_send_message(int sock, const char *user, const char *target, const char *msg){
	char INSERT[10240];
	#ifdef DEBUG
	printf("\nMessage received: %s\n\n",msg);
	#endif
	snprintf(INSERT,10239,"INSERT INTO Messages (target,source,msg) VALUES ('%s','%s','%s');",target,user,msg);
	if(mysql_real_query(conn,INSERT,strlen(INSERT))){return;}
	
}

static void register_user(const char *login,const char *hash){
	int n;
	MYSQL_RES *res;
	char SELECT[1024];
	snprintf(SELECT,1023,"SELECT * FROM Users WHERE login='%s';",login);
	mysql_query(conn,SELECT);
	res=mysql_store_result(conn);
	n=mysql_num_rows(res);
	#ifdef DEBUG
	printf("\n%s\n",SELECT);
	printf("%d users of this name registered\n\n",n);
	#endif
	mysql_free_result(res);
	if(n==0){
		snprintf(SELECT,1023,"INSERT INTO Users (login,hash) VALUES ('%s','%s');",login,hash);
		mysql_query(conn,SELECT);
	}
}


void* handler(void *a){
	struct thread_args *args=(struct thread_args*)a;
	char buffer[10240];
	const char *command;
	int number_of_rows;
	json_t *request, *json_command, *json_login,*json_hash;

	MYSQL_RES *res;

	char SELECT[1024];


	recv(args->sock,buffer,10240,0);
	request=json_loads(buffer,strlen(buffer),NULL);
	if(request){
		json_command=json_object_get(request,"cmd");
		if(json_command)command=json_string_value(json_command);
		if(strcmp(command,"register")){
			json_login=json_object_get(request,"login");
			json_hash=json_object_get(request,"hash");
			snprintf(SELECT,1023,"SELECT * FROM Users WHERE login='%s' AND hash='%s';",json_string_value(json_login),json_string_value(json_hash));
			#ifdef DEBUG
			printf("Login: %s\n",SELECT);
			#endif
			
			if(mysql_query(conn,SELECT)){
				close(args->sock);
				return NULL;
			}
			res=mysql_store_result(conn);
			if(res==NULL){close(args->sock);free(a);return NULL;}
			number_of_rows=mysql_num_rows(res);
			mysql_free_result(res);
			
			#ifdef DEBUG
			printf("N rows: %d\n",number_of_rows);
			#endif		
			
			if(number_of_rows==1){
				if(!strcmp("recv",command)){
					handle_recv_messages(args->sock,json_string_value(json_login));
				}
				if(!strcmp("send",command)){
					handle_send_message(
							args->sock,
							json_string_value(json_login),
							json_string_value(json_object_get(request,"target")),
							json_string_value(json_object_get(request,"msg"))
							);
				}
			}

		}else{
			json_login=json_object_get(request,"login");
			json_hash=json_object_get(request,"hash");
			register_user(json_string_value(json_login),json_string_value(json_hash));
		}
	}
	close(args->sock);
	free(a);
	return NULL;
}

#undef DEBUG

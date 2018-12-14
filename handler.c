#include "handler.h"
#include <sys/socket.h>
#include <jansson.h>
#include <string.h>
#include <mariadb/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define HOST "localhost"
#define USER "handler"
#define PASS "1234"
#define DATABASE "Komunikator"
#define PORT 3306
#define SOCKET NULL
#define OPTIONS 0


static void handle_recv_messages(int sock,const char *user){
	int i;
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char msg[10240];
	char *lbracket="[";
	char *rbracket="]";
	char SELECT[1024];
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,HOST,USER,PASS,DATABASE,PORT,SOCKET,OPTIONS))return;
	snprintf(SELECT,1023,"SELECT source,msg FROM Messages WHERE target='%s';",user);
	if(mysql_query(conn,SELECT)){mysql_close(conn);return;}
	res=mysql_use_result(conn);
	if(res==NULL){mysql_close(conn);return;}
	send(sock,lbracket,strlen(lbracket),0);
	i=0;
	while(row=mysql_fetch_row(res)){
		if(i)send(sock,",",strlen(","),0);
		snprintf(msg,10239,"{\"from\":\"%s\",\"msg\":\"%s\" }",row[0],row[1]);
		send(sock,msg,strlen(msg),0);
	}
	send(sock,rbracket,strlen(rbracket),0);
	mysql_free_result(res);
	mysql_close(conn);
}

static void handle_send_message(int sock, const char *user, const char *target, const char *msg){
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	
	char INSERT[10240];

	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,HOST,USER,PASS,DATABASE,PORT,SOCKET,OPTIONS))return;
	snprintf(INSERT,10239,"INSERT INTO Messages (target,source,msg) VALUES ('%s','%s','%s');",target,user,msg);
	if(mysql_query(conn,INSERT)){mysql_close(conn);return;}
	mysql_close(conn);
}

static void register_user(const char *login,const char *hash){
	int n;
	MYSQL *conn;
	MYSQL_RES *res;
	char SELECT[1024];
	conn=mysql_init(NULL);
	snprintf(SELECT,1023,"SELECT * FROM Users WHERE login='%s';",login);
	mysql_query(conn,SELECT);
	res=mysql_use_result(conn);
	n=mysql_num_rows(res);
	mysql_free_result(res);
	if(n==0){
		snprintf(SELECT,1023,"INSERT INTO Users (login,hash) VALUES ('%s','%s');",login,hash);
		mysql_query(conn,SELECT);
	}
	mysql_close(conn);
}


void* handler(void *a){
	struct thread_args *args=(struct thread_args*)a;
	char buffer[10240];
	char *login;
	char *hash;
	char *command;
	char user_id[10];
	int number_of_rows;
	json_t *request, *json_command, *json_login,*json_hash;

	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	char SELECT[1024];


	recv(args->sock,buffer,10240,0);
	request=json_loads(buffer,strlen(buffer),NULL);
	if(request){
		json_command=json_object_get(request,"cmd");
		if(json_command)command=json_string_value(json_command);
		if(strcmp(command,"register")){
			json_login=json_object_get(request,"login");
			if(json_login)login=json_string_value(json_login);
			json_hash=json_object_get(request,"hash");
			if(json_hash)hash=json_string_value(json_hash);
			if(hash!=NULL&&login!=NULL){
				conn=mysql_init(NULL);
				if(!mysql_real_connect(conn,HOST,USER,PASS,DATABASE,PORT,SOCKET,OPTIONS)){
					close(args->sock);
					return NULL;
				}
				snprintf(SELECT,1023,"SELECT id FROM Users WHERE login='%s' AND hash='%s';",login,hash);
				if(mysql_query(conn,SELECT)){
					close(args->sock);
					return NULL;
				}
				res=mysql_use_result(conn);
				if(res==NULL){close(args->sock);return NULL;}
				number_of_rows=mysql_num_rows(res);
				if(number_of_rows==1){
					row=mysql_fetch_row(res);
					strcpy(user_id,row[0]);
					if(!strcmp("recv",command)){
						handle_recv_messages(args->sock,login);
					}
					if(!strcmp("send",command)){
						handle_send_message(
								args->sock,
								login,
								json_string_value(json_object_get(request,"target")),
								json_string_value(json_object_get(request,"msg"))
								);
					}
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

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

#def PORT 20000

struct thread_args{
int sock;
};

static void* handler(void*arg)
{
	char bufor [2048], login [20], password [30];
	struct thread_args *a=(thread_args*)arg;
	int sock=a->sock;
	recv(sock, bufor, 2048, 0);
}

int main (void) {
int gniazdo;
struct sockaddr_in adr,nadawca;
socklen_t dl= sizeof (struct sockaddr_in);

gniazdo = socket (PF_INET, SOCK_STREAM, 0);
adr.sin_family = AF_INET;
adr.sin_port = htons(PORT);
adr.sin_addr.s_addr = INADDR_ANY;
if (bind(gniazdo, (struct sockaddr*) &adr,
sizeof(adr)) < 0){
return 1;
}
if(listen(gniazdo, 1000) < 0) {
return 1;
}


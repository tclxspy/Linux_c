
#include <pthread.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include "wrap.h"

#define SRV_PORT				8888
#define MAXLINE 				8192


struct s_info {
	struct sockaddr_in clt_addr;
	int cfd;
};


void * do_work(void * arg)
{
	struct s_info * ts = (struct s_info *)arg;
	int n, i;
	char buf[MAXLINE];
	char str[INET_ADDRSTRLEN];

	while (1) {
		n = Read(ts->cfd, buf, MAXLINE);

		if (n == 0) {
			printf("the client %d closed...\n", ts->cfd);
			break;
		}

		printf("received from %s at PORT %d\n", inet_ntop(AF_INET, & (*ts).clt_addr.sin_addr, str, sizeof(str)), 
			ntohs((*ts).clt_addr.sin_port));

		for (i = 0; i < n; i++)
			buf[i] = toupper(buf[i]);

		Write(STDOUT_FILENO, buf, n);
		Write(ts->cfd, buf, n);
	}

	close(ts->cfd);

	return (void *) 0;
}


int main(int argc, char * argv[])
{
	struct sockaddr_in srv_addr, clt_addr;
	socklen_t clt_addr_len;
	int lfd, cfd;

	struct s_info ts[256];
	int i = 0;
	pthread_t tid;

	bzero(&srv_addr, sizeof(srv_addr));

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(SRV_PORT);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	lfd = Socket(AF_INET, SOCK_STREAM, 0);

	int opt = 1;

	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	Bind(lfd, (struct sockaddr *) &srv_addr, sizeof(srv_addr));

	Listen(lfd, 128);

	clt_addr_len = sizeof(clt_addr);

	printf("accepting client connect ...\n");

	while (1) {
		cfd = Accept(lfd, (struct sockaddr *) &clt_addr, &clt_addr_len);
		ts[i].clt_addr = clt_addr;
		ts[i].cfd = cfd;

		pthread_create(&tid, NULL, do_work, (void *) &ts[i]);
		pthread_detach(tid);
		i++;
	}

	return 0;
}



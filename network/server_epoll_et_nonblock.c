
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "wrap.h"

#define MAXLINE 				10
#define SVR_PORT				9000
#define OPEN_MAX				10


int main(int argc, char * argv[])
{
	struct sockaddr_in srv_addr, clt_addr;
	socklen_t clt_addr_len;
	char buf[BUFSIZ], clt_ip[INET_ADDRSTRLEN];

	int listenfd = Socket(AF_INET, SOCK_STREAM, 0); //socket();

	int opt = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bzero(&srv_addr, sizeof(srv_addr)); 			//clear addr
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(SVR_PORT);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	Bind(listenfd, (struct sockaddr *) &srv_addr, sizeof(srv_addr)); //bind();
	Listen(listenfd, 128);							//listen();

	int efd = epoll_create(OPEN_MAX);				//create epoll

	if (efd == -1)
		sys_err("epoll create error");

	struct epoll_event tep, ep[OPEN_MAX];

	clt_addr_len = sizeof(clt_addr);
	int connfd = Accept(listenfd, (struct sockaddr *) &clt_addr, &clt_addr_len); //accept();
	printf("received from %s at port %d\n", inet_ntop(AF_INET, &clt_addr.sin_addr, clt_ip, sizeof(clt_ip)), 
		ntohs(clt_addr.sin_port));

	int flag = fcntl(connfd, F_GETFL);
	fcntl(connfd, F_SETFL, flag | O_NONBLOCK);
    
	tep.events = EPOLLIN | EPOLLET; 				//epoll + ET + non block + loop
	//tep.events = EPOLLIN; 				        //default is LT
	tep.data.fd = connfd;

	int res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep); //add connfd to epoll

	if (res == -1)
		sys_err("epoll ctl error");

	while (1) {
		int nready = epoll_wait(efd, ep, OPEN_MAX, -1); //epoll wait

		if (nready == -1)
			sys_err("epoll wait error");

		if (ep[0].data.fd == connfd) {
			//non block, busy to loop
			while (1) {
				int count = read(connfd, buf, MAXLINE / 2);

				if (count == 0)
					break;

				write(STDOUT_FILENO, buf, count);
			}
		}
	}

	close(connfd);
	close(listenfd);
    close(efd);
	return 0;
}



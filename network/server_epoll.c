
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/epoll.h>
#include "wrap.h"

#define SVR_PORT				9527
#define OPEN_MAX				3000


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
	tep.events = EPOLLIN;
	tep.data.fd = listenfd;

	int res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep); //add listenfd to epoll

	if (res == -1)
		sys_err("epoll ctl error");

	int i;

	while (1) {
		int nready = epoll_wait(efd, ep, OPEN_MAX, -1); //epoll wait

		if (nready == -1)
			sys_err("epoll wait error");

		for (i = 0; i < nready; i++) {
			if (!ep[i].events & EPOLLIN)
				continue;

			if (ep[i].data.fd == listenfd) {
				clt_addr_len = sizeof(clt_addr);
				int connfd = Accept(listenfd, (struct sockaddr *) &clt_addr, &clt_addr_len); //accept();

				tep.events = EPOLLIN;
				tep.data.fd = connfd;

				res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep); //add connfd to epoll

				if (res == -1)
					sys_err("epoll ctl error");

				printf("received from %s at port %d\n", inet_ntop(AF_INET, 
					&clt_addr.sin_addr, 
					clt_ip, 
					sizeof(clt_ip)), 
					ntohs(clt_addr.sin_port));

			}
			else {
				int fd = ep[i].data.fd;
				int count = Read(fd, buf, sizeof(buf)); //read();

				if (count == 0) {
					res = epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL); //delete fd of epoll

					if (res == -1)
						sys_err("epoll ctl error");

					close(fd);
					printf("client fd %d closed\n", fd);
				}
				else if (count < 0) {
					epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL); //delete fd of epoll
					close(fd);
					sys_err("read error");
				}
				else if (count > 0) {
					for (i = 0; i < count; i++)
						buf[i] = toupper(buf[i]); //toupper();

					Write(STDOUT_FILENO, buf, count);
					Write(fd, buf, count);			//write();
				}
			}
		}
	}

	close(listenfd);								//close();
	close(efd);
	return 0;
}



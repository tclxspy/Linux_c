
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <ctype.h>

#include "wrap.h"

#define MAXLINE 				80
#define SVR_PORT				9527
#define OPEN_MAX				1024


int main(int argc, char * argv[])
{
	struct sockaddr_in srv_addr, clt_addr;
	socklen_t clt_addr_len;
	char buf[BUFSIZ], clt_ip[INET_ADDRSTRLEN];
	ssize_t n, count;

	struct pollfd client[OPEN_MAX]; //client fd
	int sockfd, connfd, i, maxi;
	int listenfd = Socket(AF_INET, SOCK_STREAM, 0); 	//socket();

	int opt = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bzero(&srv_addr, sizeof(srv_addr)); 			//clear addr
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(SVR_PORT);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	Bind(listenfd, (struct sockaddr *) &srv_addr, sizeof(srv_addr)); //bind();

	Listen(listenfd, 128);							//listen();

	//set client[] default -1
	for (i = 0; i < OPEN_MAX; i++) {
		client[i].fd = -1;
	}

	//set listenfd to client[0]
	client[0].fd = listenfd;
	client[0].events = POLLIN;

    maxi = 0;

	while (1) {
		//poll, wait to connect
		int nready = poll(client, maxi + 1, -1);

		//response for listen, new client
		if (client[0].revents & POLLIN) {
			clt_addr_len = sizeof(clt_addr);

			//get client fd
			connfd = Accept(listenfd, (struct sockaddr *) &clt_addr, &clt_addr_len);

			printf("received from %s at port %d\n", inet_ntop(AF_INET, &clt_addr.sin_addr, clt_ip, sizeof(clt_ip)), 
				ntohs(clt_addr.sin_port));

			//add client fd into client[]
			for (i = 1; i < OPEN_MAX; i++) {
				if (client[i].fd < 0) {
					client[i].fd = connfd;
					break;
				}
			}

			if (i == OPEN_MAX)
				sys_err("too many clients");

			//set event for new client
			client[i].events = POLLIN;

			if (i > maxi)
				maxi = i;

			if (--nready <= 0) //only one request
				continue;
		}

		//get data from client
		for (i = 1; i <= maxi; i++) {
			if ((sockfd = client[i].fd) < 0)
				continue;

			if (client[i].revents & POLLIN) {
				n = Read(sockfd, buf, MAXLINE);

				if (n < 0) {
					//get RST
					if (errno == ECONNRESET) {
						printf("client[%d] aborted connection\n", i);
						close(sockfd);
						client[i].fd = -1;
					}
					else 
						sys_err("read error");
				}
				else if (n == 0) {
					printf("client[%d] closed connection\n", i);
					close(sockfd);
					client[i].fd = -1;
				}
				else {
					for (count = 0; count < n; count++)
						buf[count] = toupper(buf[count]);

					writen(sockfd, buf, n);
				}

				if (--nready <= 0)
					break;
			}
		}
	}

	return 0;
}



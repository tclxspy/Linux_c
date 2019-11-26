
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "wrap.h"

#define MAXLINE 				10
#define SRV_PORT				9000
#define SRV_ADDR				"127.0.0.1"


int main(int argc, char * argv[])
{
	struct sockaddr_in srv_addr;
	char buf[MAXLINE];
	int i;
	char ch = 'a';

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(SRV_PORT);
	inet_pton(AF_INET, SRV_ADDR, &srv_addr.sin_addr);

	connect(sockfd, (struct sockaddr *) &srv_addr, sizeof(srv_addr));

	while (1) {
		for (i = 0; i < MAXLINE / 2; i++)
			buf[i] = ch;

		buf[i - 1] = '\n';
		ch++;

		for (; i < MAXLINE; i++)
			buf[i] = ch;

		buf[i - 1] = '\n';
		ch++;
		write(sockfd, buf, sizeof(buf));
		sleep(3);
	}

	close(sockfd);

	return 0;
}



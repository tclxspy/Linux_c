#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define SVR_PORT 9527
#define SVR_IP "127.0.0.1"

void sys_err(const char *str)
{
    perror(str);
    exit(1);
}

int main(int argc, char *argv[])
{
    char buf[BUFSIZ];
    struct sockaddr_in svr_addr;

    bzero(&svr_addr, sizeof(svr_addr));

    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(SVR_PORT);
    inet_pton(AF_INET, SVR_IP, &svr_addr.sin_addr.s_addr);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);	//socket();
    if (sockfd == -1) {
	sys_err("socket error");
    }

    int ret = connect(sockfd, (struct sockaddr *) &svr_addr, sizeof(svr_addr));	//connect();
    if (ret == -1) {
	sys_err("socket error");
    }

    while (1) {
	fgets(buf, sizeof(buf), stdin);
	sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &svr_addr, sizeof(svr_addr));	//write
	int n = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, 0);	//read
	write(STDOUT_FILENO, buf, n);
	sleep(1);
    }

    close(sockfd);		//close();

    return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>

#define SVR_PORT 9527

void sys_err(const char *str)
{
    perror(str);
    exit(1);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in svr_addr, clit_addr;
    char buf[BUFSIZ], clit_ip[1024];

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);	//socket();
    if (sockfd == -1) {
	sys_err("socket error");
    }

    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(SVR_PORT);
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(sockfd, (struct sockaddr *) &svr_addr, sizeof(svr_addr));	//bind();
    if (ret == -1) {
	sys_err("bind error");
    }

    socklen_t clit_addr_len = sizeof(clit_addr);

    int i;
    while (1) {
	int n = recvfrom(sockfd, buf, BUFSIZ, 0, (struct sockaddr *) &clit_addr, &clit_addr_len);	//read

	printf("client ip : %s port : %d\n",
	       inet_ntop(AF_INET, &clit_addr.sin_addr.s_addr, clit_ip,
			 sizeof(clit_ip)), ntohs(clit_addr.sin_port));

	write(STDOUT_FILENO, buf, n);

	for (i = 0; i < n; i++) {
	    buf[i] = toupper(buf[i]);	//toupper();
	}
	sendto(sockfd, buf, n, 0, (struct sockaddr *) &clit_addr, clit_addr_len);	//write
    }

    close(sockfd);		//close();

    return 0;
}

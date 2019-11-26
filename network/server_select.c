#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/select.h>
#include "wrap.h"

#define SVR_PORT 9527

int main(int argc, char *argv[])
{
    int listenfd, clt_fd[FD_SETSIZE], clt_fd_num, max_clt_fd_num = -1;

    struct sockaddr_in srv_addr, clt_addr;
    socklen_t clt_addr_len;

    char buf[BUFSIZ], clt_ip[INET_ADDRSTRLEN];

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);	//socket();

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&srv_addr, sizeof(srv_addr));	//clear addr
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(SVR_PORT);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(listenfd, (struct sockaddr *) &srv_addr, sizeof(srv_addr));	//bind();

    Listen(listenfd, 128);	//listen();

    fd_set rset, allset;
    int maxfd = listenfd;

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for (clt_fd_num = 0; clt_fd_num < FD_SETSIZE; clt_fd_num++) {//set default clt_fd[] = -1
	clt_fd[clt_fd_num] = -1;
    }

    int i;
    while (1) {
	rset = allset;
	int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);//select
	if (nready < 0)
	    sys_err("select error");

	if (FD_ISSET(listenfd, &rset)) {	//new connnection
	    clt_addr_len = sizeof(clt_addr);
	    int connfd = Accept(listenfd, (struct sockaddr *) &clt_addr, &clt_addr_len);	//accept();

	    for (clt_fd_num = 0; clt_fd_num < FD_SETSIZE; clt_fd_num++) {//save connfd to clt_fd[]
		if (clt_fd[clt_fd_num] < 0) {
		    clt_fd[clt_fd_num] = connfd;
		    break;
		}
	    }

	    if (clt_fd_num == FD_SETSIZE) {//check max
		sys_err("too many clients\n");
	    }

	    if (max_clt_fd_num < clt_fd_num)//get max
		max_clt_fd_num = clt_fd_num;

	    printf("received from %s at port %d\n", inet_ntop(AF_INET,
							      &clt_addr.
							      sin_addr,
							      clt_ip,
							      sizeof(clt_ip)),
		   ntohs(clt_addr.sin_port));

	    FD_SET(connfd, &allset);	//add new connect fd to allset

	    if (maxfd < connfd)
		maxfd = connfd;

	    if (nready == 1)	//only one request
		continue;
	}

	for (clt_fd_num = 0; clt_fd_num <= max_clt_fd_num; clt_fd_num++) {//loop read request
	    int fd = clt_fd[clt_fd_num];
	    if (fd < 0)
		continue;

	    if (FD_ISSET(fd, &rset)) {
		int count = Read(fd, buf, sizeof(buf));	//read();
		if (count == 0) {
		    close(fd);
		    FD_CLR(fd, &allset);
		    clt_fd[clt_fd_num] = -1;//return to default
		} else if (count == -1) {
		    sys_err("read error");
		} else if (count > 0) {
		    for (i = 0; i < count; i++)
			buf[i] = toupper(buf[i]);	//toupper();

		    Write(STDOUT_FILENO, buf, count);
		    Write(fd, buf, count);	//write();
		}

		if (nready == 1)	//only one request
		    continue;
	    }
	}
    }

    close(listenfd);		//close();
    return 0;
}

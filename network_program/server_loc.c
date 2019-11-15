#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <sys/un.h>
#include "wrap.h"

#define SVR_ADDR "svr_loc.socket"

int main(int argc, char *argv[])
{
    struct sockaddr_un svr_addr, clit_addr;
    char buf[BUFSIZ];

    int listenfd = Socket(AF_UNIX, SOCK_STREAM, 0);	//socket();

    bzero(&svr_addr, sizeof(svr_addr));

    svr_addr.sun_family = AF_UNIX;
    strcpy(svr_addr.sun_path, SVR_ADDR);

    int len =
	offsetof(struct sockaddr_un, sun_path) + strlen(svr_addr.sun_path);//offsetof();

    unlink(SVR_ADDR);//unlink socketfile
    Bind(listenfd, (struct sockaddr *) &svr_addr, len);	//bind();

    Listen(listenfd, 20);	//listen();

    printf("Accept...\n");
    int i, size;
    while (1) {
	len = sizeof(clit_addr);
	int clit_fd = Accept(listenfd, (struct sockaddr *) &clit_addr, (socklen_t *)&len);	//accept();

	len -= offsetof(struct sockaddr_un, sun_path);
	clit_addr.sun_path[len] = '\0';
	printf("client bind filename %s\n", clit_addr.sun_path);

	while((size = Read(clit_fd, buf, sizeof(buf))) > 0) {	//read();
	    write(STDOUT_FILENO, buf, size);

	    for (i = 0; i < size; i++) {
		buf[i] = toupper(buf[i]);	//toupper();
	    }
	    Write(clit_fd, buf, size);	//write();
	}
	close(clit_fd);
    }

    close(listenfd);		//close();

    return 0;
}

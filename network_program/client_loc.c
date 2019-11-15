#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>
#include <string.h>
#include <stddef.h>
#include "wrap.h"

#define SVR_ADDR "svr_loc.socket"
#define CLIT_ADDR "clit_loc.socket"

int main(int argc, char *argv[])
{
    char buf[BUFSIZ];
    struct sockaddr_un svr_addr, clit_addr;
    int clit_fd = socket(AF_UNIX, SOCK_STREAM, 0);	//socket();
/////bind client addr
    bzero(&clit_addr, sizeof(clit_addr));
    clit_addr.sun_family = AF_UNIX;
    strcpy(clit_addr.sun_path, CLIT_ADDR);

    int len = offsetof(struct sockaddr_un,
		       sun_path) + strlen(clit_addr.sun_path);	//offsetof();
    unlink(CLIT_ADDR);
    Bind(clit_fd, (struct sockaddr *) &clit_addr, len);	//need bind();

///////////////////////////////////////////////////////////////////////////////////////
/////connet server addr
    bzero(&svr_addr, sizeof(svr_addr));
    svr_addr.sun_family = AF_UNIX;
    strcpy(svr_addr.sun_path, SVR_ADDR);

    len = offsetof(struct sockaddr_un, sun_path) + strlen(svr_addr.sun_path);	//offsetof

    Connect(clit_fd, (struct sockaddr *) &svr_addr, len);	//connect();

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
	Write(clit_fd, buf, strlen(buf));	//write();
	len = Read(clit_fd, buf, sizeof(buf));	//read();
	write(STDOUT_FILENO, buf, len);
	sleep(1);
    }

    close(clit_fd);		//close();

    return 0;
}

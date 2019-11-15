#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include "wrap.h"

#define SVR_PORT 9527

int main(int argc, char *argv[])
{
    struct sockaddr_in svr_addr, clit_addr;
    socklen_t clit_addr_len;
    char buf[BUFSIZ], clit_ip[1024];

    int svr_fd = Socket(AF_INET, SOCK_STREAM, 0);//socket();

    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(SVR_PORT);
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(svr_fd, (struct sockaddr *) &svr_addr, sizeof(svr_addr));//bind();

    Listen(svr_fd, 128);//listen();

    clit_addr_len = sizeof(clit_addr);
    int clit_fd = accept(svr_fd, (struct sockaddr *) &clit_addr, &clit_addr_len);//accept();

    printf("client ip : %s port : %d\n",
		    inet_ntop(AF_INET, &clit_addr.sin_addr.s_addr, clit_ip, sizeof(clit_ip)), ntohs(clit_addr.sin_port));

    int i;
    while (1) {
	int size = Read(clit_fd, buf, sizeof(buf));//read();
	write(STDOUT_FILENO, buf, size);

	for (i = 0; i < size; i++) {
	    buf[i] = toupper(buf[i]);//toupper();
	}
	Write(clit_fd, buf, size);//write();
    }

    close(svr_fd);//close();
    close(clit_fd);

    return 0;
}

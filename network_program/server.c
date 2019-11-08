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
    int svr_fd, clit_fd, ret, i;
    struct sockaddr_in svr_addr, clit_addr;
    socklen_t clit_addr_len;
    char buf[BUFSIZ], clit_ip[1024];

    svr_fd = socket(AF_INET, SOCK_STREAM, 0);//socket();
    if (svr_fd == -1) {
	sys_err("socket error");
    }

    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(SVR_PORT);
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(svr_fd, (struct sockaddr *) &svr_addr, sizeof(svr_addr));//bind();
    if (ret == -1) {
	sys_err("bind error");
    }

    ret = listen(svr_fd, 128);//listen();
    if (ret == -1) {
	sys_err("listen error");
    }

    clit_addr_len = sizeof(clit_addr);
    clit_fd = accept(svr_fd, (struct sockaddr *) &clit_addr, &clit_addr_len);//accept();
    if (clit_fd == -1) {
	sys_err("accept error");
    }

    printf("client ip : %s port : %d\n",
		    inet_ntop(AF_INET, &clit_addr.sin_addr.s_addr, clit_ip, sizeof(clit_ip)), ntohs(clit_addr.sin_port));

    while (1) {
	ret = read(clit_fd, buf, sizeof(buf));//read();
	write(STDOUT_FILENO, buf, ret);

	for (i = 0; i < ret; i++) {
	    buf[i] = toupper(buf[i]);//toupper();
	}
	write(clit_fd, buf, ret);//write();
    }

    close(svr_fd);//close();
    close(clit_fd);

    return 0;
}

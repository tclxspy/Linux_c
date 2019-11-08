#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SVR_PORT 9527

void sys_err(const char *str)
{
    perror(str);
    exit(1);
}

int main(int argc, char *argv[])
{
    int clit_fd, ret;
    char buf[BUFSIZ];
    struct sockaddr_in svr_addr;

    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(SVR_PORT);
    inet_pton(AF_INET, "127.0.0.1", &svr_addr.sin_addr.s_addr);

    clit_fd = socket(AF_INET, SOCK_STREAM, 0);//socket();
    if (clit_fd == -1) {
	sys_err("socket error");
    }

    ret = connect(clit_fd, (struct sockaddr *) &svr_addr, sizeof(svr_addr));//connect();
    if (ret == -1) {
	sys_err("socket error");
    }

    while (1) {
	write(clit_fd, "hello\n", 6);//write();
	ret = read(clit_fd, buf, sizeof(buf));//read();
	write(STDOUT_FILENO, buf, ret);
	sleep(1);
    }

    close(clit_fd);//close();

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "wrap.h"

#define SVR_PORT 9527

int main(int argc, char *argv[])
{
	char buf[BUFSIZ];
	struct sockaddr_in svr_addr;

	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(SVR_PORT);
	inet_pton(AF_INET, "127.0.0.1", &svr_addr.sin_addr.s_addr);

	int clit_fd = Socket(AF_INET, SOCK_STREAM, 0);	//socket();

	Connect(clit_fd, (struct sockaddr *) &svr_addr, sizeof(svr_addr));	//connect();

	while (fgets(buf, sizeof(buf), stdin) != NULL) {
		Write(clit_fd, buf, strlen(buf));	//write();

		int size = Read(clit_fd, buf, sizeof(buf));	//read();
		write(STDOUT_FILENO, buf, size);
		sleep(1);
	}

	close(clit_fd);		//close();

	return 0;
}

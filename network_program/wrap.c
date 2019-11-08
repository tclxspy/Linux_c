#include "wrap.h"

void sys_err(const char *str)
{
    perror(str);
    exit(-1);
}

int Socket(int domain, int type, int protocol)
{
    int value = socket(domain, type, protocol);
    if (value < 0) {
	sys_err("socket error");
    }
    return value;
}

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int value = bind(sockfd, addr, addrlen);
    if (value < 0) {
	sys_err("bind error");
    }
    return value;
}

int Listen(int sockfd, int backlog)
{
    int value = listen(sockfd, backlog);
    if (value < 0) {
	sys_err("listen error");
    }
    return value;
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int value;
  again:
    if ((value = accept(sockfd, addr, addrlen)) < 0) {
	if ((errno == ECONNABORTED) || (errno == EINTR)) {
	    goto again;
	} else {
	    sys_err("accept error");
	}
    }
    return value;
}

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int value = connect(sockfd, addr, addrlen);
    if (value < 0) {
	sys_err("socket error");
    }
    return value;
}

ssize_t Readn(int sockfd, void *vptr, size_t n)
{
    size_t n_left;
    ssize_t n_read;
    char *ptr;

    ptr = vptr;
    n_left = n;

    while (n_left > 0) {
	if ((n_read = read(sockfd, ptr, n_left)) < 0) {
	    if (errno == EINTR)
		n_read = 0;
	    else
		return -1;
	} else if (n_read == 0)
	    break;

	n_left -= n_read;
	ptr += n_read;
    }

    return n - n_left;
}

ssize_t Writen(int sockfd, const void *vptr, size_t n)
{
    size_t n_left;
    ssize_t n_written;
    const char *ptr;

    ptr = vptr;
    n_left = n;
    while(n_left > 0) {
	if ((n_written = write(sockfd, ptr, n_left)) <= 0) {
	    if (n_written < 0 && errno == EINTR)
		n_written = 0;
	    else
		return -1;
	}
	n_left -= n_written;
	ptr += n_written;
    }
    return n;
}

static ssize_t my_read(int sockfd, char *ptr)
{
    static int read_cnt;
    static char *read_ptr;
    static char read_buf[100];

    if (read_cnt <= 0) {
      again:
	if ((read_cnt = read(sockfd, read_buf, sizeof(read_buf))) < 0) {
	    if (errno == EINTR)
		goto again;
	    return -1;
	} else if (read_cnt == 0)
	    return 0;
	read_ptr = read_buf;
    }
    read_cnt--;
    *ptr = *read_ptr++;

    return 1;
}

ssize_t Readline(int sockfd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char c, *ptr;
    ptr = vptr;

    for (n = 1; n < maxlen; n++) {
	if ((rc = my_read(sockfd, &c)) == 1) {
	    *ptr++ = c;
	    if (c == '\n')
		break;
	} else if (rc == 0) {
	    *ptr = 0;
	    return n - 1;
	} else
	    return -1;
    }
    *ptr = 0;

    return n;
}

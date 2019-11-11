#ifndef _WRAP_H_
#define _WRAP_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>

void sys_err(const char *str);
int Socket(int domain, int type, int protocol);
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Listen(int sockfd, int backlog);
int Accept(int sockfd, struct sockaddr *addr, socklen_t * addrlen);
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

ssize_t Read(int sockfd, void *ptr, size_t nbytes);
ssize_t Write(int sockfd, const void *buf, size_t nbytes);
ssize_t readn(int sockfd, void *vptr, size_t n);
ssize_t writen(int sockfd, const void *vptr, size_t n);
ssize_t readline(int sockfd, void *vptr, size_t maxlen);

#endif

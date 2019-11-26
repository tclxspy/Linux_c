
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <arpa/inet.h>
#include "wrap.h"
#include <time.h>

#define SRV_PORT				9527
#define SRV_ADDR				"127.0.0.1"


void read_cb(struct bufferevent * bev, void * arg)
{
	char buf[BUFSIZ] = {
		0
	};

	//get data from server
	size_t len = bufferevent_read(bev, buf, sizeof(buf));

	if (len == 0) {
		return;
	}
	else if (len < 0) {
		sys_err("read server error");
	}

	//printf("server send: %s\n", buf);
	write(STDOUT_FILENO, buf, strlen(buf));

	sleep(1);

	return;
}


void write_cb(struct bufferevent * bev, void * arg)
{
	//printf("i am client. the call back is unuseful.\n");
	return;
}


void event_cb(struct bufferevent * bev, short what, void * arg)
{
	if (what & BEV_EVENT_EOF) {
		printf("connection closed\n");
	}
	else if (what & BEV_EVENT_ERROR) {
		printf("something error\n");
	}
	else if (what & BEV_EVENT_CONNECTED) {
		printf("connected server...\n");
		return;
	}

	bufferevent_free(bev);
	printf("free bufferevent\n");

	return;
}


void read_terminal(evutil_socket_t fd, short what, void * arg)
{
	//get data from terminal
	char buf[BUFSIZ] = {
		0
	};

	//read data from terminal
	int len = read(fd, buf, sizeof(buf));

	if (len == 0) {
		return;
	}
	else if (len < 0) {
		sys_err("read terminal error");
	}

	//send data to server
	struct bufferevent * bev = (struct bufferevent *)arg;
	bufferevent_write(bev, buf, len);

	return;
}


int main(int argc, char * argv[])
{
	struct event_base * base = event_base_new();

	//create socket, buffer event, add into event_base
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	struct bufferevent * bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	struct sockaddr_in srv_addr;
	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(SRV_PORT);
	inet_pton(AF_INET, SRV_ADDR, &srv_addr.sin_addr.s_addr);

	//connect server
	bufferevent_socket_connect(bev, (struct sockaddr *) &srv_addr, sizeof(srv_addr));

	//set call back function
	bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);

	//enable read buffer, default EV_READ is disable
	bufferevent_enable(bev, EV_READ | EV_WRITE);

	//create event: input client data from terminal
	struct event * ev = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST, read_terminal, bev);

	//add event into event_base
	event_add(ev, NULL);

	//loop
	event_base_dispatch(base);

	//free event
	event_free(ev);

	//free event_base
	event_base_free(base);

	//close fd
	close(fd);

	return 0;
}




#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include "wrap.h"

#define SRV_PORT				9527


void read_cb(struct bufferevent * bev, void * arg)
{
	char buf[BUFSIZ] = {
		0
	};

	//read client by server
	size_t len = bufferevent_read(bev, buf, sizeof(buf));

	if (len == 0) {
		return;
	}
	else if (len < 0) {
		sys_err("read client error");
	}

	//printf("client send: %s\n", buf);
	write(STDOUT_FILENO, buf, strlen(buf));

	char * buffer = "server feedback: receive your data, thanks.\n";

	//send data to client
	bufferevent_write(bev, buffer, strlen(buffer));

	sleep(1);

	return;
}


void write_cb(struct bufferevent * bev, void * arg)
{
	//printf("i am server. i success to feedback. and called by write_cb function.\n");
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

	bufferevent_free(bev);
	printf("free bufferevent\n");

	return;
}


void cb_listener(struct evconnlistener * listener, evutil_socket_t fd, struct sockaddr * addr, int socklen, 
	void * arg)
{
	printf("connect new client...\n");

	//get event_base
	struct event_base * base = (struct event_base *)
	arg;

	//create buffer event, add into event_base
	struct bufferevent * bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

	//set call back function
	bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);

	//enable read buffer, default EV_READ is disable
	bufferevent_enable(bev, EV_READ | EV_WRITE);

	return;
}


static void signal_cb(evutil_socket_t sig, short events, void * user_data)
{
	struct event_base * base = user_data;
	struct timeval delay = {
		1, 0
	};

	printf("Caught an interrupt signal; exiting cleanly in one seconds.\n");
	event_base_loopexit(base, &delay);
}


int main(int argc, char * argv[])
{
	//server addr
	struct sockaddr_in srv_addr;
	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(SRV_PORT);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//create event_base
	struct event_base * base = event_base_new();

	//create socket, bind, listener
	struct evconnlistener * listener = evconnlistener_new_bind(base, cb_listener, base, 
		LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 36, (struct sockaddr *) &srv_addr, sizeof(srv_addr));

	if (!listener) {
		fprintf(stderr, "Could not create a listener!\n");
		return 1;
	}

	struct event * signal_event = evsignal_new(base, SIGINT, signal_cb, (void *) base);

	if (!signal_event || event_add(signal_event, NULL) < 0) {
		fprintf(stderr, "Could not create/add a signal event!\n");
		return 1;
	}

	//loop
	event_base_dispatch(base);

	//free evconnlistener
	evconnlistener_free(listener);

	//free event base
	event_base_free(base);

	return 0;
}



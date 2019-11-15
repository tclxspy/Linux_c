
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <event2/event.h>
#include "wrap.h"


void write_cb(evutil_socket_t fd, short what, void * arg)
{
	char buf[BUFSIZ];

	static int num = 0;

	sprintf(buf, "hello, libevent! -- %d\n", num++);
	write(fd, buf, strlen(buf) + 1);

	sleep(1);

	return;
}


int main(int argc, char * argv[])
{
	int fd = open("event_myfifo", O_WRONLY | O_NONBLOCK);

	if (fd == -1) {
		sys_err("open error");
	}

	//create event_base
	struct event_base * base = event_base_new();

	//create event
	struct event * ev = event_new(base, fd, EV_WRITE | EV_PERSIST, write_cb, NULL);

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


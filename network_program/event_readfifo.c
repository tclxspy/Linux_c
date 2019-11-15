
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "wrap.h"
#include <event2/event.h>


void read_cb(evutil_socket_t fd, short what, void * arg)
{
	char buf[BUFSIZ] = {
		0
	};

	//read fd
	int len = read(fd, buf, sizeof(buf));

	if (len == 0) {
		return;
	}
	else if (len < 0) {
		sys_err("read fifo error");
	}

	printf("what event = %s\n", what & EV_READ ? "read": "no read");
	printf("data len = %d, buf = %s\n", len, buf);

	return;
}


int main(int argc, char * argv[])
{
	unlink("event_myfifo");
	mkfifo("event_myfifo", 0644);

	int fd = open("event_myfifo", O_RDONLY | O_NONBLOCK);

	if (fd == -1) {
		sys_err("open error");
	}

	//create event_base
	struct event_base * base = event_base_new();

	//create event
	struct event * ev = event_new(base, fd, EV_READ | EV_PERSIST, read_cb, NULL);

	//add event to enent_base
	event_add(ev, NULL);

	//loop
	event_base_dispatch(base);

	//free evet
	event_free(ev);

	//free event_base
	event_base_free(base);

	//close fd
	close(fd);
    
	return 0;
}



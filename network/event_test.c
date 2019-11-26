#include <stdio.h>
#include <stdlib.h>
#include <event2/event.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	struct event_base *base = event_base_new();
	const char **buf_1;
	buf_1 = event_get_supported_methods();

	int i;
	for(i = 0; i < 10; i++) {
		if(buf_1[i] == NULL)
			break;
		printf("event_get_supported_methods: buf_1[%d] = %s\n", i, buf_1[i]);
	}

	const char *buf_2 = event_base_get_method(base);
	printf("event_base_get_method: %s\n", buf_2);

	return 0;
}
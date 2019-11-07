#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

void *tfn(void *arg)
{
	printf("thread: pid = %d, tid = %lu\n", getpid(), pthread_self());
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t tid;

	printf("man : pid = %d, tid = %lu\n", getpid(), pthread_self());

	int ret = pthread_create(&tid, NULL, tfn, NULL);
	if(ret != 0) {
		perror("pthead_create error");
	}
	sleep(1);
	return 0;
}

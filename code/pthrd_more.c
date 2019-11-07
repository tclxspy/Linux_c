#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

void *tfn(void *arg)
{
	int i = (int)arg;
	sleep(i);
	printf("-- i am %dth thread: pid = %d, tid = %lu\n", i, getpid(), pthread_self());
	return NULL;
}

int main(int argc, char *argv[])
{
	int i, ret;
	pthread_t tid;
	
	for(i = 0; i < 5; i++)
	{
		ret = pthread_create(&tid, NULL, tfn, (void *)i);
		if(ret != 0) {
			perror("pthead_create error");
		}
	}

	sleep(i);
	printf("man : pid = %d, tid = %lu\n", getpid(), pthread_self());
	return 0;
}

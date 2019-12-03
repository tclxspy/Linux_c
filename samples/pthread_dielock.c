
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

pthread_mutex_t mutex_1, mutex_2;
int var_1, var_2;


void * tfn_1(void * arg)
{
	int count = 0;
	int i = (int)

	arg;
	sleep(i);

	while (1) {
		printf("--i am %dth thread: pid = %d, tid = %lu\n", i, getpid(), pthread_self());
		pthread_mutex_lock(&mutex_1);
		printf("tfn_1 : ");
		sleep(rand() % 1);
		printf("var_1 = %d ", var_1);

		sleep(1);

		pthread_mutex_lock(&mutex_2);
		printf("var_2 = %d\n", var_2);

		pthread_mutex_unlock(&mutex_2);
		pthread_mutex_unlock(&mutex_1);

		sleep(1);

		if (++count == 5) {
			pthread_exit((void *) 1);
		}
	}
}


void * tfn_2(void * arg)
{
	int count = 0;
	int i = (int)

	arg;
	sleep(i);

	while (1) {
		printf("--i am %dth thread: pid = %d, tid = %lu\n", i, getpid(), pthread_self());
		pthread_mutex_lock(&mutex_2);
		printf("tfn_2 : ");
		sleep(rand() % 1);
		printf("var_2 = %d ", var_2);

		sleep(1);

		pthread_mutex_lock(&mutex_1);
		printf("var_1 = %d\n", var_1);

		pthread_mutex_unlock(&mutex_1);
		pthread_mutex_unlock(&mutex_2);

		sleep(1);

		if (++count == 5) {
			pthread_exit((void *) 2);
		}
	}
}



int main(int argc, char * agrv[])
{
	int ret;
	void * reval_1 = NULL;
	void * reval_2 = NULL;
	pthread_t tid_1, tid_2;

	var_1 = 100;
	var_2 = 0;

	ret = pthread_mutex_init(&mutex_1, NULL);

	if (ret != 0) {
		fprintf(stderr, "pthread_mutex_init mutex_1 error");
	}

	ret = pthread_mutex_init(&mutex_2, NULL);

	if (ret != 0) {
		fprintf(stderr, "pthread_mutex_init mutex_2 error");
	}

	ret = pthread_create(&tid_1, NULL, tfn_1, (void *) 1);

	if (ret != 0) {
		fprintf(stderr, "pthread_creat 1 error");
	}

	ret = pthread_create(&tid_2, NULL, tfn_2, (void *) 2);

	if (ret != 0) {
		fprintf(stderr, "pthread_creat 2 error");
	}

	pthread_join(tid_1, &reval_1);
	pthread_join(tid_2, &reval_2);
	printf("pthread_join reval_1 = %d\n", (int) reval_1);
	printf("pthread_join reval_2 = %d\n", (int) reval_2);

	pthread_mutex_destroy(&mutex_1);
	pthread_mutex_destroy(&mutex_2);

	sleep(1);

	return 0;

}



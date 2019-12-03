
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_NUM 				5

sem_t start_num, produce_num;
int queue[MAX_NUM];


void * producer(void * arg)
{
	int i = 0;

	while (1) {
		sem_wait(&produce_num); 					//wait to produce, produce_num--
		queue[i] = rand() % 1000 + 1;				//produce		
		printf("producer id: %lu, queue[%d] = %d\n", 
			pthread_self(), i, queue[i]);
		sem_post(&start_num);						//start_num++

		i = (i + 1) % MAX_NUM;
		sleep(rand() % 1);
	}

	return NULL;
}


void * consumer(void * arg)
{
	int i = 0;

	while (1) {
		sem_wait(&start_num);						//wait to consume, start_num--
		printf("---consumer id: %lu, queue[%d] = %d\n", //consumer
		pthread_self(), i, queue[i]);
		queue[i] = 0;
		sem_post(&produce_num); 					//produce_num++

		i = (i + 1) % MAX_NUM;
		sleep(rand() % 3);
	}

	return NULL;
}


int main(int argc, char * argv[])
{
	pthread_t pid, cid;

	sem_init(&start_num, 0, 0);
	sem_init(&produce_num, 0, MAX_NUM);

	pthread_create(&pid, NULL, producer, NULL);
	pthread_create(&cid, NULL, consumer, NULL);

	pthread_join(pid, NULL);
	pthread_join(cid, NULL);

	sem_destroy(&start_num);
	sem_destroy(&produce_num);

	return 0;
}



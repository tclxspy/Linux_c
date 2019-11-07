#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

struct msg {
    struct msg *next;
    int num;
};

struct msg *head;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t has_data = PTHREAD_COND_INITIALIZER;

void *producer(void *arg)
{
    while (1) {
	struct msg *mp = malloc(sizeof(struct msg));
	mp->num = rand() % 1000 + 1;	//1~1000
	printf("--produce mp->num = %d\n", mp->num);

	pthread_mutex_lock(&mutex);	//lock, and modify shared_var
	mp->next = head;
	head = mp;
	pthread_mutex_unlock(&mutex);
	pthread_cond_signal(&has_data);	//broadcast

	sleep(rand() % 3);
    }
    return NULL;
}

void *consumer(void *arg)
{
    while (1) {
	struct msg *mp;

	pthread_mutex_lock(&mutex);	//lock,
	while (head == NULL) {	//wait condition, unlock
	    pthread_cond_wait(&has_data, &mutex);
	}			//lock, after exit this conditioon

	mp = head;
	head = mp->next;
	pthread_mutex_unlock(&mutex);	//unlock,

	printf("------consumer id : %lu, mp->num = %d\n",
	       pthread_self(), mp->num);
	free(mp);

	sleep(rand() % 3);
    }
    return NULL;
}


int main(int argc, char *argv[])
{
    pthread_t pid, cid[3];
    int i;

    srand(time(NULL));

    pthread_create(&pid, NULL, producer, NULL);
    for (i = 0; i < 3; i++) {
	pthread_create(&cid[i], NULL, consumer, NULL);
    }

    pthread_join(pid, NULL);
    for (i = 0; i < 3; i++) {
	pthread_join(cid[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&has_data);

    return 0;
}

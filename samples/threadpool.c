
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <signal.h>


#define DEFAULT_TIME			10		/*10s检测一次*/
#define MIN_WAIT_TASK_NUM		10		/*如果queue_size > MIN_WAIT_TASK_NUM 添加新的线程到线程池*/ 
#define DEFAULT_THREAD_VARY 	10		/*每次创建和销毁线程的个数*/
#define true					1
#define false					0

typedef struct {
	void * (*function) (void *);					/*函数指针，回调函数*/
	void * arg; 									/*上面函数的参数*/
} threadpool_task_t; /*任务结构体*/


typedef struct {
	pthread_mutex_t lock;							/*用于锁住当前这个结构体体taskpoll*/
	pthread_mutex_t thread_counter; 				/*记录忙状态线程个数*/

	pthread_cond_t queue_not_full; /*当任务队列满时，添加任务的线程阻塞，等待此
									条件变量*/
	pthread_cond_t queue_not_empty; 				/*任务队列里不为空时，通知等待任务的线程*/
	pthread_t * threads;							/*保存工作线程tid的数组*/
	pthread_t adjust_tid;							/*管理线程tid*/
	threadpool_task_t * task_queue; 				/*任务队列*/
	int min_thr_num;								/*线程组内默认最小线程数*/
	int max_thr_num;								/*线程组内默认最大线程数*/
	int live_thr_num;								/*当前存活线程个数*/
	int busy_thr_num;								/*忙状态线程个数*/
	int wait_exit_thr_num;							/*要销毁的线程个数*/
	int queue_front;								/*队头索引下标*/
	int queue_rear; 								/*队未索引下标*/
	int queue_size; 								/*队中元素个数*/
	int queue_max_size; 							/*队列中最大容纳个数*/
	int shutdown;									/*线程池使用状态，true或false*/
} threadpool_t;


void * threadpool_thread(void * threadpool)
{
	threadpool_t * pool = (threadpool_t *)threadpool;
	threadpool_task_t task;

	while (true) {
		/* Lock must be taken to wait on conditional variable */
		/*刚创建出线程，等待任务队列里有任务，否则阻塞等待任务队列里有任务后再唤醒
			接收任务*/
		pthread_mutex_lock(& (pool->lock));

		while ((pool->queue_size == 0) && (!pool->shutdown)) {
			printf("thread 0x%x is waiting\n", (unsigned int) pthread_self());
			pthread_cond_wait(& (pool->queue_not_empty), & (pool->lock));

			/*清除指定数目的空闲线程，如果要结束的线程个数大于0，结束线程*/
			if (pool->wait_exit_thr_num > 0) {
				pool->wait_exit_thr_num--;

				/*如果线程池里线程个数大于最小值时可以结束当前线程*/
				if (pool->live_thr_num > pool->min_thr_num) {
					printf("thread 0x%x is exiting\n", (unsigned int) pthread_self());
					pool->live_thr_num--;
					pthread_mutex_unlock(& (pool->lock));
					pthread_exit(NULL);
				}
			}
		}

		/*如果指定了true，要关闭线程池里的每个线程，自行退出处理*/
		if (pool->shutdown) {
			pthread_mutex_unlock(& (pool->lock));
			printf("thread 0x%x is exiting\n", (unsigned int) pthread_self());
			pthread_exit(NULL);
		}

		/*从任务队列里获得任务，是一个出队操作*/
		task.function = pool->task_queue[pool->queue_front].function;
		task.arg = pool->task_queue[pool->queue_front].arg;

		//出队，模拟环形队列
		pool->queue_front = (pool->queue_front + 1) % pool->queue_max_size;
		pool->queue_size--;

		/*通知可以有新的任务添加进来*/
		pthread_cond_broadcast(& (pool->queue_not_full));

		//任务取出后，释放线程池锁
		pthread_mutex_unlock(& (pool->lock));

		/*执行任务*/
		printf("thread 0x%x start working\n", (unsigned int) pthread_self());
		pthread_mutex_lock(& (pool->thread_counter));
		pool->busy_thr_num++;						/*忙状态线程数加1*/
		pthread_mutex_unlock(& (pool->thread_counter));

		(* (task.function)) (task.arg); 			/*执行回调函数任务*/

		//task.function(task.arg);								 /*执行回调函数任务*/

		/*任务结束处理*/
		printf("thread 0x%x end working\n", (unsigned int) pthread_self());
		pthread_mutex_lock(& (pool->thread_counter));
		pool->busy_thr_num--;						/*忙状态数减1*/
		pthread_mutex_unlock(& (pool->thread_counter));
	}

	pthread_exit(NULL);

	//return (NULL);
}


int pthreadpool_is_thread_alive(pthread_t tid)
{
	int kill_rc = pthread_kill(tid, 0);

	if (kill_rc == ESRCH) {
		return false;
	}

	return true;
}


int threadpool_free(threadpool_t * pool)
{
	if (pool == NULL) {
		return - 1;
	}

	if (pool->task_queue) {
		free(pool->task_queue);
	}

	if (pool->threads) {
		free(pool->threads);
		pthread_mutex_lock(& (pool->lock));
		pthread_mutex_destroy(& (pool->lock));
		pthread_mutex_lock(& (pool->thread_counter));
		pthread_mutex_destroy(& (pool->thread_counter));
		pthread_cond_destroy(& (pool->queue_not_empty));
		pthread_cond_destroy(& (pool->queue_not_full));
	}

	free(pool);
	pool = NULL;

	return 0;
}


//管理者线程
void * threadpool_adjust_thread(void * threadpool)
{
	int i;
	threadpool_t * pool = (threadpool_t *)threadpool;

	while (!pool->shutdown) {
		sleep(DEFAULT_TIME);						/*延时10秒*/
		pthread_mutex_lock(& (pool->lock));
		int queue_size = pool->queue_size;			//获取任务数
		int live_thr_num = pool->live_thr_num;		//获取存活线程数
		pthread_mutex_unlock(& (pool->lock));

		pthread_mutex_lock(& (pool->thread_counter));
		int busy_thr_num = pool->busy_thr_num;		//获取忙线程数
		pthread_mutex_unlock(& (pool->thread_counter));

		/*任务数大于最小线程池个数，并且存活的线程数少于最大线程个数时，创建新线程
			，如10<20<100*/
		if (queue_size >= MIN_WAIT_TASK_NUM && live_thr_num < pool->max_thr_num) {
			pthread_mutex_lock(& (pool->lock));
			int add = 0;

			/*一次增加DEFAULT_THREAD个线程*/
			for (i = 0; i < pool->max_thr_num && add < DEFAULT_THREAD_VARY && pool->live_thr_num < pool->max_thr_num;
				 i++) {
				if (pool->threads[i] == 0 || !pthreadpool_is_thread_alive(pool->threads[i])) {
					pthread_create(& (pool->threads[i]), NULL, threadpool_thread, (void *) pool);
					add++;
					pool->live_thr_num++;
				}
			}

			pthread_mutex_unlock(& (pool->lock));
		}

		/*销毁多余的空闲线程, 忙线程数*2 小于存活线程数，且存活线程数大于最小线程数�
			��*/
		if ((busy_thr_num * 2) < live_thr_num && live_thr_num > pool->min_thr_num) {
			/*一次销毁DEFAULT_THREAD个线程*/
			pthread_mutex_lock(& (pool->lock));
			pool->wait_exit_thr_num = DEFAULT_THREAD_VARY;
			pthread_mutex_unlock(& (pool->lock));

			for (i = 0; i < DEFAULT_THREAD_VARY; i++) {
				/*通知处在空闲状态的线程,他们会自动终止*/
				pthread_cond_signal(& (pool->queue_not_empty));
			}
		}
	}

	return NULL;
}


threadpool_t * threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size)
{
	int i;
	threadpool_t * pool = NULL;

	do {
		if ((pool = (threadpool_t *) malloc(sizeof(threadpool_t))) == NULL) {
			printf("malloc threadpool fail");
			break;									//跳出do while
		}

		pool->min_thr_num = min_thr_num;
		pool->max_thr_num = max_thr_num;
		pool->busy_thr_num = 0;
		pool->live_thr_num = min_thr_num;
		pool->queue_size = 0;
		pool->queue_max_size = queue_max_size;
		pool->queue_front = 0;
		pool->queue_rear = 0;
		pool->shutdown = false; 					//不关闭线程池

		//根据最大线程上限数，给工作线程数组开辟空间，并清零
		pool->threads = (pthread_t *)malloc(sizeof(pthread_t) *max_thr_num);

		if (pool->threads == NULL) {
			printf("malloc threads fail");
			break;
		}
        //需要乘以max_thr_num
		memset(pool->threads, 0, sizeof(pool->threads) *max_thr_num);

		//给任务队列开辟空间
		pool->task_queue = (threadpool_task_t *)malloc(sizeof(threadpool_task_t) *queue_max_size);

		if (pool->task_queue == NULL) {
			printf("malloc task_queue fail");
			break;
		}

		//初始化互斥锁、条件变量
		if (pthread_mutex_init(& (pool->lock), NULL) != 0 ||
			 pthread_mutex_init(& (pool->thread_counter), NULL) != 0 ||
			 pthread_cond_init(& (pool->queue_not_empty), NULL) != 0 ||
			 pthread_cond_init(& (pool->queue_not_full), NULL) != 0) {
			printf("init the lock or cond fail");
			break;
		}

		/*创建 min_thr_num 个work thread */
		for (i = 0; i < min_thr_num; i++) {
			pthread_create(& (pool->threads[i]), NULL, threadpool_thread, (void *) pool);
			printf("start thread 0x%x...\n", (unsigned int) pool->threads[i]);
		}

		//创建管理者线程
		pthread_create(& (pool->adjust_tid), NULL, threadpool_adjust_thread, (void *) pool);
		return pool;

	}
	while(0);

	//为什么是while(0)，以便出错break循环，不用执行后面的代码
	threadpool_free(pool);							//前面代码调用失败时，释放poll存储空间

	return NULL;
}


int threadpool_add(threadpool_t * pool, void * (*function) (void * arg), void * arg)
{
	pthread_mutex_lock(& (pool->lock));

	//为真，队列已经满，调wait阻塞
	while ((pool->queue_size == pool->queue_max_size) && (!pool->shutdown)) {
		pthread_cond_wait(& (pool->queue_not_full), & (pool->lock));
	}

	if (pool->shutdown) {
		pthread_cond_broadcast(& (pool->queue_not_empty));
		pthread_mutex_unlock(& (pool->lock));
	}

	/*添加任务到任务队列里,清空参数arg*/
	if (pool->task_queue[pool->queue_rear].arg != NULL) {
		free(pool->task_queue[pool->queue_rear].arg);
		pool->task_queue[pool->queue_rear].arg = NULL;
	}

	//添加任务到任务队列里
	pool->task_queue[pool->queue_rear].function = function;
	pool->task_queue[pool->queue_rear].arg = arg;

	//队尾指针移到，模拟环形
	pool->queue_rear = (pool->queue_rear + 1) % pool->queue_max_size;
	pool->queue_size++;

	/*添加完任务后，任务队列不为空，唤醒等待处理任务的线程*/
	pthread_cond_signal(& (pool->queue_not_empty));
	pthread_mutex_unlock(& (pool->lock));

	return 0;
}


//线程池中的线程，模拟处理任务
void * threadpool_process(void * arg)
{
	printf("thread 0x%x working on task %d\n ", (unsigned int) pthread_self(), * (int *) arg);
	sleep(1);
	printf("task %d is end\n", * (int *) arg);

	return NULL;
}


int threadpool_destroy(threadpool_t * pool)
{
	int i;

	if (pool == NULL) {
		return - 1;
	}

	pool->shutdown = true;

	/*先销毁管理线程*/
	pthread_join(pool->adjust_tid, NULL);

	for (i = 0; i < pool->live_thr_num; i++) {
		/*通知所有的空闲线程*/
		pthread_cond_broadcast(& (pool->queue_not_empty));
	}

	//回收空闲线程
	for (i = 0; i < pool->live_thr_num; i++) {
		pthread_join(pool->threads[i], NULL);
	}

	threadpool_free(pool);

	return 0;
}


int main(int argc, char * argv[])
{
	threadpool_t * thp = threadpool_create(3, 100, 100); //线程池里最小3个线程，最大100个，队列最大100

	printf("pool inited\n");

	//int * num = (int *)malloc(sizeof(int) * 20);
	int num[20];
	int i;

	for (i = 0; i < 20; i++) {
		num[i] = i;
		printf("add task %d\n", i);
		threadpool_add(thp, threadpool_process, (void *) &num[i]);
	}

	sleep(10);
	threadpool_destroy(thp);

	return 0;
}


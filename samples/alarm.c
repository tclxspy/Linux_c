
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>


void sig_handler(int sig_num)
{
	printf("signal action %d \n", sig_num);
	exit(0);
}


int main(void)
{
	int i;

	signal(SIGALRM, sig_handler);
	alarm(1);

	//after 1s, to execute sig_handler function
	for (i = 0; ; i++)
		printf("%d\n", i);

	return 0;
}



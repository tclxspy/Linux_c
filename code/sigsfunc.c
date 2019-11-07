#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void print_set(sigset_t * set)
{
    int i;
    for (i = 0; i < 32; i++) {
	if (sigismember(set, i))
	    putchar('1');
	else
	    putchar('0');
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    int ret = 0;
    sigset_t set, oldset, pedset;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);

    ret = sigprocmask(SIG_BLOCK, &set, &oldset);

    if (ret == -1) {
		printf("sigprocmask error");
		return -1;
    }

    while (1) {
		ret = sigpending(&pedset);
		if (ret == -1) {
	    	printf("sigpending error");
	   		return -1;
		}

		print_set(&pedset);
		sleep(1);
    }
    return 0;
}

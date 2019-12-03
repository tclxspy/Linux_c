
#include <stdio.h>
#include <signal.h>
#include <unistd.h>


int main(int argc, char * argv[])
{
	pid_t pid = fork();

	if (pid > 0) {
		printf("parent, pid = %d\n", getpid());

		while (1);
	}
	else if (pid == 0) {
		printf("child pid = %d, ppid = %d\n", getpid(), getppid());
		sleep(2);
		kill(getppid(), SIGSEGV);
	}

	return 0;
}



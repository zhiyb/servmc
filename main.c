#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include "cmd.h"
#include "exec.h"

int main(int argc, char *argv[])
{
	int ret = exec_server(NULL);
	if (ret != -EAGAIN)
		return ret;

	cmd_init();

	// Setup file descriptors
	fd_set rfds, arfds;
	FD_ZERO(&arfds);
	FD_SET(cmd_rfd(), &arfds);
	FD_SET(exec_stdout_rfd(), &arfds);

loop:	// Wait for reading data
	memcpy(&rfds, &arfds, sizeof(arfds));
	ret = select(exec_stdout_rfd() + 1, &rfds, NULL, NULL, NULL);
	if (ret < 0)
		;
	else if (ret == 0)
		goto loop;
	if (FD_ISSET(cmd_rfd(), &rfds)) {
		cmd_process();
	}
	if (FD_ISSET(exec_stdout_rfd(), &rfds)) {
		if (exec_stdout_process() == EOF)
			goto quit;
	}
	goto loop;

quit:
	cmd_quit();
	exec_quit();
	return 0;
}

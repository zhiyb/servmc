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
#include "net.h"
#include "exec.h"
#include "backup.h"
#include "update.h"
#include "config.h"

int main(int argc, char *argv[])
{
	fd_set rfds;
	cmd_init();
	net_init();

	// Initial steps
	update();
	int err = exec_server(SERVER_PATH, "18w11a.jar");
	if (err)
		fprintf(stderr, "%s: Error starting: %s\n",
				__func__, strerror(err));

loop:	// Setup file descriptors
	FD_ZERO(&rfds);
	FD_SET(cmd_rfd(), &rfds);
	int nfds = cmd_rfd();
	int fdout = exec_rfd(0), fderr = exec_rfd(1);
	if (fdout >= 0)
		FD_SET(fdout, &rfds);
	nfds = nfds >= fdout ? nfds : fdout;
	if (fderr >= 0)
		FD_SET(fderr, &rfds);
	nfds = nfds >= fderr ? nfds : fderr;

	// Wait for reading data
	int ret = select(nfds + 1, &rfds, NULL, NULL, &(struct timeval){
			.tv_sec = TICK_INTERVAL, .tv_usec = 0});
	if (ret < 0) {
		fprintf(stderr, "%s: select() failed\n", __func__);
		goto quit;
	} else if (ret == 0) {
		backup_tick();
		goto loop;
	}
	if (FD_ISSET(cmd_rfd(), &rfds))
		cmd_process();
	int quit = 0;
	if (FD_ISSET(fdout, &rfds))
		if (exec_process(0) == EOF)
			quit = 1;
	if (FD_ISSET(fderr, &rfds))
		if (exec_process(1) == EOF)
			quit = 1;
	if (quit)
		exec_quit();
	if (!cmd_shutdown())
		goto loop;

quit:
	cmd_quit();
	exec_quit();
	return 0;
}

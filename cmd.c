#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "exec.h"
#include "net.h"
#include "update.h"
#include "backup.h"
#include "cmd.h"

static void cmd_line(char *line)
{
	static const char *delim = " \n";
	add_history(line);

	// Send strings to server process
	if (*line != '!') {
		if (exec_status() < 0)
			fprintf(stderr, "%s: JAR is not running\n",
					__func__);
		else
			exec_write_stdin(line, 0);
		goto ret;
	}

	// Process special commands
	char *cmd = strtok(line, delim);
	if (strcmp(cmd, "!exec") == 0) {
		printf("%s:%u exec\n", __func__, __LINE__);
	} else if (strcmp(cmd, "!jar") == 0) {
		const char *dir = strtok(NULL, delim);
		const char *jar = strtok(NULL, delim);
		int err = exec_server(dir, jar);
		if (err)
			fprintf(stderr, "%s: Error starting: %s\n",
					__func__, strerror(err));
	} else if (strcmp(cmd, "!update") == 0) {
		update();
	} else if (strcmp(cmd, "!backup") == 0) {
		backup_now();
	} else {
		fprintf(stderr, "%s: Unknown command: %s\n",
				__func__, line);
	}

ret:
	free(line);
}

void cmd_init()
{
#if 0
	// Handling SIGINT for gracefully exit
	struct sigaction act = {
	};
	sigaction(SIGINT, NULL, NULL);
#endif

	// Setup readline
	//rl_getc_function = getc;
	rl_callback_handler_install(NULL, (rl_vcpfunc_t *)&cmd_line);
	using_history();
	stifle_history(1000);
}

int cmd_rfd()
{
	return fileno(stdin);
}

void cmd_process()
{
	rl_callback_read_char();
}

void cmd_quit()
{
	rl_callback_handler_remove();
}

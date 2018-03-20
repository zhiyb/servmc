#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "exec.h"
#include "cmd.h"

static void cmd_line(char *line)
{
	add_history(line);
	exec_write_stdin(line);
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "net.h"
#include "cmd.h"
#include "exec.h"
#include "update.h"
#include "backup.h"
#include "restart.h"
#include "monitor.h"
#include "config.h"

static int shutdown = 0;

static void cmd_line(char *line)
{
	static const char *delim = " \n";
	if (!*line)
		return;
	add_history(line);

	// Send strings to server process
	if (*line != '!') {
		if (exec_status() < 0)
			cmd_printf(CLR_ERROR, "%s: Server is not running\n",
					__func__);
		else if (!monitor_server_status())
			cmd_printf(CLR_ERROR, "%s: Server is not ready for "
					"accepting commands\n", __func__);
		else
			exec_write_stdin(line, 0);
		goto ret;
	}

	// Process special commands
	char *cmd = strtok(line, delim);
	if (strcmp(cmd, "!restart") == 0) {
		restart_schedule();
	} else if (strcmp(cmd, "!update") == 0) {
		update();
	} else if (strcmp(cmd, "!backup") == 0) {
		backup_now();
	} else if (strcmp(cmd, "!stop") == 0) {
		if (exec_status() >= 0) {
			cmd_printf(CLR_MESSAGE, "%s: Stopping server\n",
					__func__);
			exec_write_stdin(CMD_SHUTDOWN, ECHO_CMD);
		}
	} else if (strcmp(cmd, "!shutdown") == 0) {
		cmd_printf(CLR_MESSAGE, "%s: Shutting down\n", __func__);
		shutdown = 1;
		if (exec_status() >= 0)
			exec_write_stdin(CMD_SHUTDOWN, ECHO_CMD);
	} else {
		cmd_printf(CLR_ERROR, "%s: Unknown command: %s\n",
				__func__, line);
	}

ret:
	free(line);
}

void cmd_init()
{
#if 0
	// Handling SIGINT for graceful exit
	struct sigaction act = {
	};
	sigaction(SIGINT, NULL, NULL);
#endif

	// Setup readline
	//rl_getc_function = getc;
	rl_callback_handler_install(CLR_INPUT "servmc> ",
			(rl_vcpfunc_t *)&cmd_line);
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
	rl_crlf();
	rl_callback_handler_remove();
}

int cmd_shutdown()
{
	// Exit gracefully after server shutdown
	return exec_status() < 0 ? shutdown : 0;
}

void cmd_external(const char *colour)
{
	printf("\33[2K\r\e[0m%s", colour);
	fflush(stdout);
}

int cmd_printf(const char *colour, const char *fmt, ...)
{
	printf("\33[2K\r\e[0m%s", colour);
	va_list args;
	va_start(args, fmt);
	int res = vprintf(fmt, args);
	va_end(args);
	if (!RL_ISSTATE(RL_STATE_DONE))
		rl_forced_update_display();
	return res;
}

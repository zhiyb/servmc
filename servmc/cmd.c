#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "net.h"
#include "cmd.h"
#include "web.h"
#include "exec.h"
#include "update.h"
#include "restart.h"
#include "monitor.h"
#include "config.h"
#include "mon/backup.h"

static int shutdown = 0;

static void cmd_help()
{
	cmd_printf(CLR_EXTERNAL,
		"!restart:  Restart the server when no players online\n"
		"!shutdown: Shutdown the server when no players online\n"
		"!stop:     Stop the server immediately, then restart\n"
		"!quit:     Shutdown the server immediately\n"
		"!update:   Check for server version updates\n"
		"!backup:   Start a new backup\n"
		"!help:     This help message\n");
}

void cmd_line(char *line, int echo)
{
	static const char *delim = " \n";
	if (!*line)
		return;
	add_history(line);
	if (echo)
		cmd_printf(CLR_INPUT, "%s%s\n", INPUT_PROMPT, line);
	else
		web_message_f(CLR_INPUT, "%s%s\n", INPUT_PROMPT, line);

	// Send strings to server process
	if (*line != '!') {
		if (exec_status() < 0)
			cmd_printf(CLR_ERROR, "%s: Server is not running\n",
					__func__);
		else if (!monitor_server_status())
			cmd_printf(CLR_ERROR, "%s: Server is not ready for "
					"accepting commands\n", __func__);
		else
			exec_write_stdin(__func__, line, 0);
		goto ret;
	}

	// Process special commands
	char *cmd = strtok(line, delim);
	if (strcmp(cmd, "!restart") == 0) {
		restart_schedule();
	} else if (strcmp(cmd, "!shutdown") == 0) {
		cmd_printf(CLR_MESSAGE, "%s: Scheduled server shutdown\n",
				__func__);
		shutdown = 1;
		restart_schedule();
	} else if (strcmp(cmd, "!stop") == 0) {
		if (exec_status() < 0)
			cmd_printf(CLR_ERROR, "%s: Server is not running\n",
					__func__);
		else
			cmd_printf(CLR_MESSAGE, "%s: Stopping server\n",
					__func__);
		exec_stop();
	} else if (strcmp(cmd, "!quit") == 0) {
		cmd_printf(CLR_MESSAGE, "%s: Shutting down\n", __func__);
		shutdown = 1;
		exec_stop();
	} else if (strcmp(cmd, "!update") == 0) {
		update();
	} else if (strcmp(cmd, "!backup") == 0) {
		backup_now();
	} else if (strcmp(cmd, "!help") == 0) {
		cmd_help();
	} else {
		cmd_printf(CLR_ERROR, "%s: Unknown command: %s\n",
				__func__, line);
	}

ret:
	free(line);
}

static void cmd_readline(char *line)
{
	cmd_line(line, 0);
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
	rl_callback_handler_install(CLR_INPUT INPUT_PROMPT,
			(rl_vcpfunc_t *)&cmd_readline);
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
	cmd_external("");
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
	int ret = vprintf(fmt, args);
	va_end(args);
	va_start(args, fmt);
	web_message(colour, ret + 1, fmt, args);
	va_end(args);
	if (!RL_ISSTATE(RL_STATE_DONE))
		rl_forced_update_display();
	return ret;
}

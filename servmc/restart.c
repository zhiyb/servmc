#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <regex.h>
#include "cmd.h"
#include "exec.h"
#include "update.h"
#include "monitor.h"
#include "config.h"

static struct {
	struct monitor_t *mon_logout, *mon_list;
	time_t schedule;
} status = {NULL, NULL, (time_t)-1};

static void restart_now()
{
	status.schedule = (time_t)-1;
	if (cmd_shutdown())
		return;
	if (!update_current()) {
		cmd_printf(CLR_ERROR, "%s: Version info not available\n",
				__func__);
		return;
	}

	const char *ver = update_current();
	char jar[strlen(ver) + 4 + 1];
	sprintf(jar, "%s.jar", ver);
	int err = exec_server(SERVER_PATH, jar);
	if (err)
		cmd_printf(CLR_ERROR, "%s: Error starting server: %s\n",
				__func__, strerror(err));
}

void restart()
{
	if (cmd_shutdown())
		return;
	// Scheduled restart if the server stopped unexpectedly
	if (!update_pending(1)) {
		status.schedule = time(NULL) + RESTART_INTERVAL;
		cmd_printf(CLR_MESSAGE, "%s: Server restart scheduled at %s",
				__func__, ctime(&status.schedule));
	} else {
		restart_now();
	}
}

void restart_tick()
{
	if (status.schedule == (time_t)-1)
		return;
	// Wait until scheduled time passed
	if (time(NULL) - status.schedule < 0)
		return;
	restart_now();
}

static void restart_logout(struct monitor_t *mp, const char *str)
{
	// Disable callback
	monitor_enable(status.mon_logout, 0);
	// Check number of online players again
	exec_write_stdin(__func__, CMD_PLAYERS, ECHO_CMD);
	monitor_enable(status.mon_list, 1);
}

static void restart_list(struct monitor_t *mp, const char *str)
{
	// Disable callback
	monitor_enable(status.mon_list, 0);
	// Extract number of online players
	static regex_t regex = {.re_nsub = 0};
	if (regex.re_nsub == 0)
		if (regcomp(&regex, REGEX_PLAYERS, REG_EXTENDED | REG_NEWLINE))
			cmd_printf(CLR_ERROR, "%s: Cannot compile regex\n",
					__func__);
	// Check number of online players
	regmatch_t match[2];
	if (regex.re_nsub && regexec(&regex, str, 2, match, 0) == 0 &&
			match[1].rm_so != -1) {
		unsigned long num = strtoul(str + match[1].rm_so, NULL, 0);
		cmd_printf(CLR_MESSAGE, "%s: %lu player(s) online\n",
				__func__, num);
		// Schedule restart when no player online
		if (num != 0) {
			monitor_enable(status.mon_logout, 1);
			exec_write_stdin(__func__, CMD_STOP_MSG
					"Server shutdown pending", ECHO_CMD);
			return;
		}
	} else {
		cmd_printf(CLR_ERROR, "%s: Regex failed\n", __func__);
	}
	// Restart immediately
	exec_stop();
}

void restart_schedule()
{
	if (!monitor_server_status()) {
		if (exec_status() >= 0)
			exec_stop();
		else
			restart_now();
		return;
	}

	// Register callbacks
	if (!status.mon_logout)
		status.mon_logout = monitor_install(REGEX_LOGOUT,
				restart_logout);
	if (!status.mon_list)
		status.mon_list = monitor_install(REGEX_PLAYERS, restart_list);
	// Check number of online players
	exec_write_stdin(__func__, CMD_PLAYERS, ECHO_CMD);
	monitor_enable(status.mon_list, 1);
}

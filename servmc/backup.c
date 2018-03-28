#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <regex.h>
#include <sys/types.h>
#include "exec.h"
#include "cmd.h"
#include "monitor.h"
#include "backup.h"
#include "config.h"

static struct {
	struct monitor_t *mon_login, *mon_list, *mon_save, *mon_line;
	time_t schedule;
} status = {NULL, NULL, NULL, NULL, (time_t)-1};

static void backup_line(struct monitor_t *mp, const char *str)
{
	backup_tick();
}

static void backup_login(struct monitor_t *mp, const char *str)
{
	if (status.schedule != (time_t)-1)
		return;
	// Schedule backup
	status.schedule = time(NULL) + BACKUP_INTERVAL;
	cmd_printf(CLR_BACKUP, "%s: Backup scheduled at %s",
			__func__, ctime(&status.schedule));
	// Enable tick callback
	monitor_enable(status.mon_line, 1);
}

static void backup_list(struct monitor_t *mp, const char *str)
{
	// Disable callback
	monitor_enable(status.mon_list, 0);
	// Extract number of online players
	static regex_t regex = {.re_nsub = 0};
	if (regex.re_nsub == 0)
		if (regcomp(&regex, REGEX_PLAYERS, REG_EXTENDED | REG_NEWLINE))
			cmd_printf(CLR_ERROR, "%s: Cannot compile regex\n",
					__func__);
	// Schedule next backup
	regmatch_t match[2];
	if (regex.re_nsub && regexec(&regex, str, 2, match, 0) == 0 &&
			match[1].rm_so != -1) {
		unsigned long num = strtoul(str + match[1].rm_so, NULL, 0);
		cmd_printf(CLR_MESSAGE, "%s: %lu player(s) online\n",
				__func__, num);
		// Schedule next backup at next player login event
		if (num == 0) {
			monitor_enable(status.mon_login, 1);
			return;
		}
	} else {
		cmd_printf(CLR_ERROR, "%s: Regex failed\n", __func__);
	}
	// Schedule next backup immediately
	backup_login(NULL, NULL);
}

static void backup_save(struct monitor_t *mp, const char *str)
{
	status.schedule = (time_t)-1;
	// Execute backup
	cmd_printf(CLR_BACKUP, "%s: Backing up\n", __func__);
	time_t tm = time(NULL);
	int ret = exec_backup();
	if (ret)
		cmd_printf(CLR_ERROR, "%s: Backup failed: %d\n", __func__, ret);
	else
		cmd_printf(CLR_BACKUP, "%s: Backup done\n", __func__);
	// Disable callback
	monitor_enable(status.mon_save, 0);

	// Send message to server
	if (!monitor_server_status())
		return;
	char msg[64];
	int ofs = snprintf(msg, sizeof(msg), "%s%s",
			ret ? CMD_SAVE_FAIL : CMD_SAVE_DONE, ctime(&tm)) - 1;
	if (ret)
		snprintf(msg + ofs, sizeof(msg) - ofs,
				" | Backup failed: %d", ret);
	else
		snprintf(msg + ofs, sizeof(msg) - ofs, " | Backup done");
	exec_write_stdin(__func__, msg, ECHO_CMD);

	// Turn on autosave
	exec_write_stdin(__func__, CMD_SAVE_ON, ECHO_CMD);
	// Check number of online players
	exec_write_stdin(__func__, CMD_PLAYERS, ECHO_CMD);
	monitor_enable(status.mon_list, 1);
}

static void backup_prepare()
{
	cmd_printf(CLR_BACKUP, "%s: Starting backup process\n", __func__);
	status.schedule = (time_t)-1;
	// Turn off autosave, save game now
	exec_write_stdin(__func__, CMD_SAVE_OFF, ECHO_CMD);
	exec_write_stdin(__func__, CMD_SAVE_ALL, ECHO_CMD);
	// Wait for save complete
	monitor_enable(status.mon_save, 1);
	// Disable unnecessary callbacks
	monitor_enable(status.mon_login, 0);
	monitor_enable(status.mon_line, 0);
}

void backup_start()
{
	// Register callbacks
	if (!status.mon_login)
		status.mon_login = monitor_install(REGEX_LOGIN, backup_login);
	if (!status.mon_list)
		status.mon_list = monitor_install(REGEX_PLAYERS, backup_list);
	if (!status.mon_save)
		status.mon_save = monitor_install(REGEX_SAVE, backup_save);
	if (!status.mon_line)
		status.mon_line = monitor_install(REGEX_LINE, backup_line);
	// Wait for player login events before scheduling backup
	monitor_enable(status.mon_login, 1);
}

void backup_now()
{
	// Reset callbacks
	monitor_enable(status.mon_login, 0);
	monitor_enable(status.mon_list, 0);
	monitor_enable(status.mon_save, 0);
	monitor_enable(status.mon_line, 0);
	// Start backup directly if server is not running
	if (!monitor_server_status())
		backup_save(NULL, NULL);
	else
		backup_prepare();
}

void backup_stop()
{
	// If backup scheduled, execute now
	if (monitor_enabled(status.mon_line) ||
			monitor_enabled(status.mon_save))
		backup_save(NULL, NULL);
	// Disable callbacks
	monitor_enable(status.mon_login, 0);
	monitor_enable(status.mon_list, 0);
	monitor_enable(status.mon_save, 0);
	monitor_enable(status.mon_line, 0);
}

void backup_tick()
{
	if (status.schedule == (time_t)-1)
		return;
	// Wait until scheduled time passed
	if (time(NULL) - status.schedule < 0)
		return;
	// Start backup process
	backup_prepare();
}

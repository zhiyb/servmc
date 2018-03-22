#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <regex.h>
#include <sys/types.h>
#include "exec.h"
#include "monitor.h"
#include "backup.h"

#define INTERVAL	(30)

#define REGEX_LOGIN	REGEX_SERVER(INFO) "[^\\s]+ joined the game$"
#define REGEX_LIST	REGEX_SERVER(INFO) \
			"There are ([0-9]+) of a max [0-9]+ players online:"
#define REGEX_SAVE	REGEX_SERVER(INFO) "Saved the game$"
#define REGEX_LINE	REGEX_SERVER(INFO)

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
	status.schedule = time(NULL) + INTERVAL;
	fprintf(stderr, "%s: Backup scheduled at %s",
			__func__, ctime(&status.schedule));
	// Install tick callback
	status.mon_line = monitor_install(REGEX_LINE, backup_line);
}

static void backup_list(struct monitor_t *mp, const char *str)
{
	// Uninstall callback
	monitor_uninstall(&status.mon_list);
	// Extract number of online players
	static regex_t regex = {.re_nsub = 0};
	if (regex.re_nsub == 0) {
		fprintf(stderr, "%s: Compiling regex\n", __func__);
		if (regcomp(&regex, REGEX_LIST, REG_EXTENDED | REG_NEWLINE))
			fprintf(stderr, "%s: Cannot compile regex\n", __func__);
	}
	// Schedule next backup
	regmatch_t match[2];
	if (regex.re_nsub && regexec(&regex, str, 2, match, 0) == 0 &&
			match[1].rm_so != -1) {
		unsigned long num = strtoul(str + match[1].rm_so, NULL, 0);
		fprintf(stderr, "%s: %lu player(s) online\n", __func__, num);
		// Schedule next backup at next player login event
		if (num == 0) {
			status.mon_login = monitor_install(REGEX_LOGIN,
					backup_login);
			return;
		}
	} else {
		fprintf(stderr, "%s: Regex failed\n", __func__);
	}
	// Schedule next backup immediately
	backup_login(NULL, NULL);
}

static void backup_save(struct monitor_t *mp, const char *str)
{
	// Execute backup
	fprintf(stderr, "%s: Backing up\n", __func__);
	exec_backup();
	fprintf(stderr, "%s: Backup done\n", __func__);
	// Uninstall callback
	monitor_uninstall(&status.mon_save);

	if (!monitor_server_status())
		return;
	// Turn on autosave
	exec_write_stdin("save-on", 1);
	exec_write_stdin("say Backup done.", 1);
	// Check number of online players
	exec_write_stdin("list", 1);
	status.mon_list = monitor_install(REGEX_LIST, backup_list);
}

static void backup_prepare()
{
	fprintf(stderr, "%s: Starting backup process\n", __func__);
	status.schedule = (time_t)-1;
	// Turn off autosave, save game now
	exec_write_stdin("save-off", 1);
	exec_write_stdin("save-all", 1);
	// Wait for save complete
	status.mon_save = monitor_install(REGEX_SAVE, backup_save);
	// Uninstall unnecessary callbacks
	monitor_uninstall(&status.mon_login);
	monitor_uninstall(&status.mon_line);
}

void backup_start()
{
	// Wait for player login events before scheduling backup
	status.mon_login = monitor_install(REGEX_LOGIN, backup_login);
}

void backup_now()
{
	// Reset callbacks
	monitor_uninstall(&status.mon_login);
	monitor_uninstall(&status.mon_list);
	monitor_uninstall(&status.mon_save);
	monitor_uninstall(&status.mon_line);
	// Start backup directly if server is not running
	if (!monitor_server_status())
		backup_save(NULL, NULL);
	else
		backup_prepare();
}

void backup_stop()
{
	// If backup scheduled, execute now
	if (status.mon_line || status.mon_save)
		backup_save(NULL, NULL);
	// Uninstall callbacks
	monitor_uninstall(&status.mon_login);
	monitor_uninstall(&status.mon_list);
	monitor_uninstall(&status.mon_save);
	monitor_uninstall(&status.mon_line);
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

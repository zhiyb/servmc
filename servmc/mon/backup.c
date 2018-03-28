#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <json.h>
#include "../cmd.h"
#include "../exec.h"
#include "../monitor.h"
#include "../config.h"
#include "players.h"

static struct {
	time_t schedule, last;
} data = {(time_t)-1, (time_t)-1};

static void backup_save(struct monitor_t *mp, const char *str);

static struct mon_monitor_t backup_monitors[] = {
	{REGEX_SAVE, backup_save},
	{NULL, NULL}
};

static void backup_schedule()
{
	if (data.schedule != (time_t)-1)
		return;
	// Schedule backup
	data.schedule = time(NULL) + BACKUP_INTERVAL;
	cmd_printf(CLR_BACKUP, "%s: Backup scheduled at %s",
			__func__, ctime(&data.schedule));
}

static void backup_prepare()
{
	cmd_printf(CLR_BACKUP, "%s: Starting backup process\n", __func__);
	data.schedule = (time_t)-1;
	// Turn off autosave, save game now
	exec_write_stdin(__func__, CMD_SAVE_OFF, ECHO_CMD);
	exec_write_stdin(__func__, CMD_SAVE_ALL, ECHO_CMD);
	// Wait for save complete
	monitor_enable(backup_monitors[0].mon, 1);
}

static void backup_save(struct monitor_t *mp, const char *str)
{
	data.schedule = (time_t)-1;
	// Execute backup
	cmd_printf(CLR_BACKUP, "%s: Backing up\n", __func__);
	time_t tm = time(NULL);
	int ret = exec_backup();
	if (ret)
		cmd_printf(CLR_ERROR, "%s: Backup failed: %d\n", __func__, ret);
	else
		cmd_printf(CLR_BACKUP, "%s: Backup done\n", __func__);

	// Send message to server
	if (!monitor_server_status())
		goto ret;
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

	// Schedule next backup
	if (players_online())
		backup_schedule();
	// Disable callback
ret:	monitor_enable(backup_monitors[0].mon, 0);
	if (!ret)
		data.last = time(NULL);
}

void backup_now()
{
	// Reset callbacks
	monitor_enable(backup_monitors[0].mon, 0);
	// Start backup directly if server is not running
	if (!monitor_server_status())
		backup_save(NULL, NULL);
	else
		backup_prepare();
}

void backup_players(int num)
{
	if (num) {
		backup_schedule();
	} else if (!monitor_server_status()) {
		// If backup scheduled, execute now
		if (data.schedule != (time_t)-1 ||
				monitor_enabled(backup_monitors[0].mon))
			backup_now();
	}
}

void backup_tick(const char *str)
{
	if (data.schedule == (time_t)-1)
		return;
	// Wait until scheduled time passed
	if (time(NULL) - data.schedule < 0)
		return;
	// Start backup process
	backup_prepare();
}

static struct json_object *backup_json(struct json_object *act)
{
	int active = monitor_enabled(backup_monitors[0].mon);
	time_t schedule = data.schedule;

	struct json_object *obj = json_object_new_object();
	if (active) {
		json_object_object_add(obj, "status",
				json_object_new_string("active"));
	} else if (schedule != (time_t)-1) {
		json_object_object_add(obj, "status",
				json_object_new_string("scheduled"));
		json_object_object_add(obj, "time",
				json_object_new_int64(schedule));
	} else {
		json_object_object_add(obj, "status",
				json_object_new_string("idle"));
	}
	if (data.last != (time_t)-1)
		json_object_object_add(obj, "last",
				json_object_new_int64(data.last));
	return obj;
}

struct mon_module_t mon_backup = {
	.name = "backup",
	.monitors = backup_monitors,
	.line = backup_tick,
	.json = backup_json,
};

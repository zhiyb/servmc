#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <regex.h>
#include "../cmd.h"
#include "../exec.h"
#include "../update.h"
#include "../monitor.h"
#include "../config.h"
#include "players.h"

static struct {
	time_t schedule;
	int pending;
} data = {(time_t)-1, 0};

void restart_players(int cnt)
{
	if (!data.pending)
		return;
	if (cnt) {
		// Schedule restart when no player online
		exec_write_stdin(__func__, CMD_STOP_MSG
				"Server shutdown pending", ECHO_CMD);
	} else {
		// Restart immediately
		data.pending = 0;
		exec_stop();
	}
}

static void restart_now()
{
	data.schedule = (time_t)-1;
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
		data.schedule = time(NULL) + RESTART_INTERVAL;
		cmd_printf(CLR_MESSAGE, "%s: Server restart scheduled at %s",
				__func__, ctime(&data.schedule));
	} else {
		restart_now();
	}
}

void restart_tick(const char *str)
{
	if (data.schedule == (time_t)-1)
		return;
	// Wait until scheduled time passed
	if (time(NULL) - data.schedule < 0)
		return;
	restart_now();
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
	data.pending = 1;
	restart_players(players_online());
}

static struct json_object *restart_json(struct json_object *act)
{
	int exec = exec_status();
	int ready = monitor_server_status();
	int pending = data.pending;
	time_t schedule = data.schedule;

	struct json_object *obj = json_object_new_object();
	const char *status;
	if (exec < 0) {
		if (schedule == (time_t)-1) {
			status = "stopped";
		} else {
			status = "scheduled";
			json_object_object_add(obj, "time",
					json_object_new_int64(schedule));
		}
	} else if (!ready) {
		status = "starting";
	} else if (!pending) {
		status = "running";
	} else {
		status = "pending";
	}
	json_object_object_add(obj, "status", json_object_new_string(status));
	return obj;
}

struct mon_module_t mon_restart = {
	.name = "restart",
	.json = restart_json,
};

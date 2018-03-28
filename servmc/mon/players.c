#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <json.h>
#include "../cmd.h"
#include "../exec.h"
#include "../monitor.h"
#include "../config.h"
#include "backup.h"

struct player_t {
	struct player_t *next;
	char name[];
};

static struct {
	int cnt, max;
	struct player_t *player;
} data = {0, 0, NULL};

int players_online()
{
	return data.cnt;
}

static void player_triggers()
{
	backup_players(data.cnt);
}

static struct player_t **players_find(const char *name)
{
	struct player_t **ppp = &data.player;
	while (*ppp && strcmp((*ppp)->name, name) != 0)
		ppp = &(*ppp)->next;
	return ppp;
}

static void players_login(struct monitor_t *mp, const char *str)
{
	static regex_t regex = {.re_nsub = 0};
	if (regex.re_nsub == 0)
		regcomp(&regex, REGEX_LOGIN, REG_EXTENDED | REG_NEWLINE);

	data.cnt++;

	// Extract player name
	regmatch_t match[2];
	if (!regex.re_nsub || regexec(&regex, str, 2, match, 0) != 0 ||
			match[1].rm_so < 0) {
		cmd_printf(CLR_ERROR, "%s: Regex matching failed\n", __func__);
		return;
	}

	size_t len = match[1].rm_eo - match[1].rm_so;
	struct player_t *pp = malloc(sizeof(struct player_t) + len + 1);
	memcpy(pp->name, str + match[1].rm_so, len);
	pp->name[len] = 0;
	cmd_printf(CLR_MESSAGE, "%s: Player %s logged in\n",
			__func__, pp->name);

	if (*players_find(pp->name)) {
		// Player already exists in the registry
		free(pp);
	} else {
		pp->next = data.player;
		data.player = pp;
	}

	player_triggers();
}

static void players_logout(struct monitor_t *mp, const char *str)
{
	static regex_t regex = {.re_nsub = 0};
	if (regex.re_nsub == 0)
		regcomp(&regex, REGEX_LOGOUT, REG_EXTENDED | REG_NEWLINE);

	data.cnt--;

	// Extract player name
	regmatch_t match[2];
	if (!regex.re_nsub || regexec(&regex, str, 2, match, 0) != 0 ||
			match[1].rm_so < 0) {
		cmd_printf(CLR_ERROR, "%s: Regex matching failed\n", __func__);
		return;
	}

	size_t len = match[1].rm_eo - match[1].rm_so;
	char name[len + 1];
	memcpy(name, str + match[1].rm_so, len);
	name[len] = 0;

	cmd_printf(CLR_MESSAGE, "%s: Player %s logged out\n",
			__func__, name);

	struct player_t **ppp = players_find(name);
	if (!*ppp) {
		cmd_printf(CLR_ERROR, "%s: Player %s not found in registry\n",
				__func__, name);
	} else {
		void *p = *ppp;
		*ppp = (*ppp)->next;
		free(p);
	}

	player_triggers();
}

static void players_list(struct monitor_t *mp, const char *str)
{
	static regex_t regex = {.re_nsub = 0};
	if (regex.re_nsub == 0)
		regcomp(&regex, REGEX_PLAYERS, REG_EXTENDED | REG_NEWLINE);

	data.cnt--;

	regmatch_t match[5];
	if (!regex.re_nsub || regexec(&regex, str, 5, match, 0) != 0) {
		cmd_printf(CLR_ERROR, "%s: Regex matching failed\n", __func__);
		return;
	}

	// Extract number of online players
	if (match[1].rm_so < 0) {
		cmd_printf(CLR_ERROR, "%s: Matching 1 failed\n", __func__);
		return;
	}

	unsigned long num = strtoul(str + match[1].rm_so, NULL, 0);
	cmd_printf(CLR_MESSAGE, "%s: %lu player(s) online\n", __func__, num);
	data.cnt = num;

	// Extract maximum allowed number of players
	if (match[3].rm_so < 0) {
		cmd_printf(CLR_ERROR, "%s: Matching 3 failed\n", __func__);
		goto triggers;
	}

	num = strtoul(str + match[3].rm_so, NULL, 0);
	cmd_printf(CLR_MESSAGE, "%s: Maximum %lu player(s) allowed\n",
			__func__, num);
	data.max = num;

	// Extract player list
	if (match[4].rm_so < 0) {
		cmd_printf(CLR_ERROR, "%s: Matching 4 failed\n", __func__);
		goto triggers;
	}

	size_t len = match[4].rm_eo - match[4].rm_so;
	if (len == 0)
		goto triggers;
	char *pnames = malloc(len + 1);
	memcpy(pnames, str + match[4].rm_so, len);
	pnames[len] = 0;
	cmd_printf(CLR_MESSAGE, "%s: Online players: %s\n",
			__func__, pnames);

	char *sp = pnames, *p;
	while ((p = strtok(sp, " ,"))) {
		sp = NULL;
		struct player_t **ppp = players_find(p);
		// Skip if player name found in registry
		if (*ppp)
			continue;

		size_t len = strlen(p);
		struct player_t *pp = malloc(sizeof(struct player_t) + len + 1);
		strcpy(pp->name, p);
		pp->next = data.player;
		data.player = pp;
	}
	free(pnames);

triggers:
	player_triggers();
}

static struct mon_monitor_t players_monitors[] = {
	{REGEX_LOGIN, players_login},
	{REGEX_LOGOUT, players_logout},
	{REGEX_PLAYERS, players_list},
	{NULL, NULL}
};

static void players_reset()
{
	cmd_printf(CLR_MESSAGE, "%s\n", __func__);
	data.cnt = data.max = 0;
	while (data.player) {
		void *p = data.player;
		data.player = data.player->next;
		free(p);
	}
	player_triggers();
}

static void players_ready()
{
	players_reset();
	monitor_enable(players_monitors[0].mon, 1);
	monitor_enable(players_monitors[1].mon, 1);
	monitor_enable(players_monitors[2].mon, 1);
	// Query maximum player count
	exec_write_stdin(__func__, CMD_PLAYERS, ECHO_CMD);
}

static struct json_object *players_json(struct json_object *act)
{
	if (!monitor_server_status())
		return json_object_new_object();

	json_bool list = 0;
	struct json_object *listobj;
	if (json_object_object_get_ex(act, "list", &listobj))
		list = json_object_get_boolean(listobj);

	struct json_object *obj = json_object_new_object();
	json_object_object_add(obj, "online", json_object_new_int(data.cnt));
	json_object_object_add(obj, "max", json_object_new_int(data.max));

	if (!list)
		return obj;

	struct json_object *array = json_object_new_array();
	struct player_t *pp = data.player;
	for (; pp; pp = pp->next)
		json_object_array_add(array, json_object_new_string(pp->name));
	json_object_object_add(obj, "list", array);
	return obj;
}

struct mon_module_t mon_players = {
	.name = "players",
	.monitors = players_monitors,
	.ready = players_ready,
	.stop = players_reset,
	.json = players_json,
};

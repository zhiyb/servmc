#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "../cmd.h"
#include "../monitor.h"
#include "../config.h"

struct player_t {
	struct player_t *next;
	char name[];
};

static struct {
	int cnt;
	struct player_t *player;
} data = {0, NULL};

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
	pp->next = data.player;
	data.player = pp;

	cmd_printf(CLR_MESSAGE, "%s: Player %s logged in\n",
			__func__, pp->name);
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

	struct player_t **ppp = &data.player;
	while (*ppp && strcmp((*ppp)->name, name) != 0)
		ppp = &(*ppp)->next;

	if (!ppp) {
		cmd_printf(CLR_ERROR, "%s: Player %s not found in registry\n",
				__func__, name);
		return;
	}

	void *p = *ppp;
	*ppp = (*ppp)->next;
	free(p);
}

static void players_list(struct monitor_t *mp, const char *str)
{
	cmd_printf(CLR_MESSAGE, "%s\n", __func__);
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
	data.cnt = 0;
	while (data.player) {
		void *p = data.player;
		data.player = data.player->next;
		free(p);
	}
}

static void players_ready()
{
	players_reset();
	monitor_enable(players_monitors[0].mon, 1);
	monitor_enable(players_monitors[1].mon, 1);
	monitor_enable(players_monitors[2].mon, 1);
}

struct mon_module_t mon_players = {
	.name = "players",
	.monitors = players_monitors,
	.ready = players_ready,
	.stop = players_reset,
};

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/types.h>
#include "cmd.h"
#include "monitor.h"
#include "config.h"
#include "mon/restart.h"

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof((a)[0]))

static struct monitor_t {
	struct monitor_t *next;
	regex_t regex;
	monitor_func_t func;
	int enabled;
} *monitor = NULL;

static struct {
	struct monitor_t *mon_server;
	int running;
} status = {NULL, 0};

extern struct mon_module_t mon_players;
extern struct mon_module_t mon_backup;
extern struct mon_module_t mon_restart;
extern struct mon_module_t mon_system;

static struct mon_module_t *modules[] = {
	&mon_players,
	&mon_backup,
	&mon_restart,
	&mon_system,
};

struct monitor_t *monitor_install(const char *regex, monitor_func_t func)
{
	// Compile regex
	regex_t reg;
	if (regcomp(&reg, regex, REG_EXTENDED | REG_NOSUB | REG_NEWLINE)) {
		cmd_printf(CLR_ERROR, "%s: Cannot compile regex %s\n",
				__func__, regex);
		return NULL;
	}

	// Push to linked list
	struct monitor_t **mp = &monitor;
	for (; *mp; mp = &(*mp)->next);
	struct monitor_t *p = malloc(sizeof(struct monitor_t));
	p->next = NULL;
	p->regex = reg;
	p->func = func;
	p->enabled = 0;
	*mp = p;
	return p;
}

void monitor_uninstall(struct monitor_t **p)
{
	if (!*p)
		return;
	// Find the specified pointer in the list
	struct monitor_t **mp = &monitor;
	for (; *mp && *mp != *p; mp = &(*mp)->next);
	if (*mp) {
		regfree(&(*mp)->regex);
		*mp = (*mp)->next;
	}
	// Release resources
	free(*p);
	*p = NULL;
}

void monitor_enable(struct monitor_t *p, int en)
{
	if (p)
		p->enabled = en;
}

int monitor_enabled(struct monitor_t *p)
{
	return p ? p->enabled : 0;
}

void monitor_line(const char *str)
{
	struct monitor_t *p = monitor;
	for (int i = 0; i != ARRAY_SIZE(modules); i++)
		if (modules[i]->line)
			modules[i]->line(str);
	for (; p; p = p->next)
		if (p->enabled && regexec(&p->regex, str, 0, NULL, 0) == 0)
			p->func(p, str);
}

static void server_ready(struct monitor_t *mp, const char *str)
{
	cmd_printf(CLR_MESSAGE, "%s: Server ready\n", __func__);
	status.running = 1;
	for (int i = 0; i != ARRAY_SIZE(modules); i++)
		if (modules[i]->ready)
			modules[i]->ready();
}

int monitor_server_status()
{
	return status.running;
}

void monitor_server_start()
{
	if (!status.mon_server)
		status.mon_server = monitor_install(REGEX_READY, server_ready);
	monitor_enable(status.mon_server, 1);
}

void monitor_server_stop()
{
	status.running = 0;
	monitor_enable(status.mon_server, 0);
	for (int i = 0; i != ARRAY_SIZE(modules); i++)
		if (modules[i]->stop)
			modules[i]->stop();
	for (int i = 0; i != ARRAY_SIZE(modules); i++) {
		struct mon_monitor_t *mmp = modules[i]->monitors;
		while (mmp && mmp->func) {
			monitor_enable(mmp->mon, 0);
			mmp++;
		}
	}
	restart();
}

void mon_init()
{
	for (int i = 0; i != ARRAY_SIZE(modules); i++) {
		struct mon_monitor_t *mmp = modules[i]->monitors;
		while (mmp && mmp->func) {
			mmp->mon = monitor_install(mmp->regex, mmp->func);
			mmp++;
		}
	}
}

void mon_deinit()
{
	for (int i = 0; i != ARRAY_SIZE(modules); i++) {
		struct mon_monitor_t *mmp = modules[i]->monitors;
		while (mmp && mmp->func) {
			monitor_uninstall(&mmp->mon);
			mmp++;
		}
	}
}

struct json_object *mon_json(struct json_object *act, const char *type)
{
	for (int i = 0; i != ARRAY_SIZE(modules); i++) {
		struct mon_module_t *mmp = modules[i];
		if (strcmp(mmp->name, type) == 0) {
			if (mmp->json)
				return mmp->json(act);
			return NULL;
		}
	}
	return NULL;
}

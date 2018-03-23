#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <sys/types.h>
#include "backup.h"
#include "config.h"
#include "restart.h"
#include "monitor.h"

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

struct monitor_t *monitor_install(const char *regex, monitor_func_t func)
{
	// Compile regex
	regex_t reg;
	if (regcomp(&reg, regex, REG_EXTENDED | REG_NOSUB | REG_NEWLINE)) {
		fprintf(stderr, "%s: Cannot compile regex %s\n",
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
	for (; p; p = p->next)
		if (p->enabled && regexec(&p->regex, str, 0, NULL, 0) == 0)
			p->func(p, str);
}

static void server_ready(struct monitor_t *mp, const char *str)
{
	fprintf(stderr, "%s: Server ready\n", __func__);
	status.running = 1;
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
	backup_start();
}

void monitor_server_stop()
{
	status.running = 0;
	monitor_enable(status.mon_server, 0);
	backup_stop();
	restart();
}

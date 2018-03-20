#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <sys/types.h>
#include "monitor.h"

static struct monitor_t {
	struct monitor_t *next;
	regex_t regex;
	monitor_func_t func;
} *monitor = NULL;

static struct {
	int running;
	struct monitor_t *mon_server;
} status = {
	.running = 0,
};

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
	*mp = p;
	return p;
}

void monitor_uninstall(struct monitor_t *p)
{
	// Push to linked list
	struct monitor_t **mp = &monitor;
	for (; *mp && *mp != p; mp = &(*mp)->next);
	if (*mp)
		*mp = (*mp)->next;
	free(p);
}

void monitor_line(const char *str)
{
	struct monitor_t *p = monitor;
	for (; p; p = p->next)
		if (regexec(&p->regex, str, 0, NULL, 0) == 0)
			p->func(p, str);
}

static void server_done(struct monitor_t *mp, const char *str)
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
	status.mon_server = monitor_install(REGEX_SERVER(INFO)
			"Done \\([0-9.]+s\\)!", server_done);
}

void monitor_server_stop()
{
	status.running = 0;
	monitor_uninstall(status.mon_server);
}

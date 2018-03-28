#pragma once

#include <json.h>

struct monitor_t;
typedef void (*monitor_func_t)(struct monitor_t *, const char *);

struct mon_monitor_t {
	const char *regex;
	const monitor_func_t func;
	struct monitor_t *mon;
};

struct mon_module_t {
	const char *name;
	struct mon_monitor_t *monitors;
	void (*const ready)();
	void (*const stop)();
	void (*const line)(const char *str);
	struct json_object *(*const json)(struct json_object *act);
};

struct monitor_t *monitor_install(const char *regex, monitor_func_t func);
void monitor_uninstall(struct monitor_t **p);
void monitor_enable(struct monitor_t *p, int en);
int monitor_enabled(struct monitor_t *p);
void monitor_line(const char *str);

int monitor_server_status();
void monitor_server_start();
void monitor_server_stop();

void mon_init();
void mon_deinit();
struct json_object *mon_json(struct json_object *act, const char *type);

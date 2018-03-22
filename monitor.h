#pragma once

struct monitor_t;
typedef void (*monitor_func_t)(struct monitor_t *, const char *);

struct monitor_t *monitor_install(const char *regex, monitor_func_t func);
void monitor_uninstall(struct monitor_t **p);
void monitor_enable(struct monitor_t *p, int en);
int monitor_enabled(struct monitor_t *p);
void monitor_line(const char *str);

int monitor_server_status();
void monitor_server_start();
void monitor_server_stop();

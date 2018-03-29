#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "../monitor.h"
#include "../config.h"
#include "../cmd.h"

static struct {
	struct stat_t {
		struct stat_cpu_t {
			struct stat_cpu_t *next;
			union {
				unsigned long val[8];
				struct {
					unsigned long user, nice, system, idle;
					unsigned long iowait, irq, softirq;
					unsigned long steal;
				};
			};
			unsigned long total;
			char name[];
		} *cpu;
	} * volatile prev, * volatile now, *buf;
	pthread_mutex_t lock;
	time_t schedule;
} data = {0};

static int system_perf_cpu(struct stat_t *stat)
{
	// Overall CPU usage
	FILE *fp = fopen("/proc/stat", "r");
	if (!fp)
		return -1;
	struct stat_cpu_t **cpp = &stat->cpu;
	char buf[128];
	while (fgets(buf, sizeof(buf), fp)) {
		if (strncmp(buf, "cpu", 3) != 0)
			break;
		static const char *delim = " ";
		char *save;
		// CPU name
		const char *field = strtok_r(buf, delim, &save);
		struct stat_cpu_t *cpu;
		if (*cpp && strcmp((*cpp)->name, field) == 0) {
			cpu = *cpp;
		} else {
			cpu = malloc(sizeof(struct stat_cpu_t) +
					strlen(field) + 1);
			strcpy(cpu->name, field);
		}
		// user, nice, system, idle, iowait, irq, softirq, steal
		unsigned long total = 0;
		int i;
		for (i = 0; i != 8; i++) {
			unsigned long v = strtoul(strtok_r(NULL, delim, &save),
					NULL, 10);
			cpu->val[i] = v;
			total += v;
		}
		cpu->total = total;
		// Append to list
		if (cpu != *cpp) {
			cpu->next = *cpp;
			*cpp = cpu;
		}
		cpp = &(*cpp)->next;
	}
	if (*cpp) {
		struct stat_cpu_t *p = *cpp;
		*cpp = (*cpp)->next;
		free(p);
	}

	// Process CPU usage
	return 0;
}

static void system_perf_free(struct stat_t *stat)
{
	if (!stat)
		return;
	struct stat_cpu_t *cp = stat->cpu;
	while (cp) {
		struct stat_cpu_t *p = cp;
		cp = cp->next;
		free(p);
	}
	free(stat);
}

static void system_perf_print()
{
#if 0
	// CPU usage
	struct stat_cpu_t *cpp = data.prev->cpu;
	struct stat_cpu_t *cnp = data.now->cpu;
	while (cpp && cnp) {
		static const char *desc[] = {
			"us", "sy", "ni", "id", "wa", "hi", "si", "st"};
		unsigned long dt = cnp->total - cpp->total;
		char str[128], *p = str;
#if 1
		p += sprintf(p, "%%%s: ", cnp->name);
		int i;
		for (i = 0; i != 8; i++) {
			unsigned long d = cnp->val[i] - cpp->val[i];
			float f = (float)d / (float)dt * 100.0;
			p += sprintf(p, "%0.1f %s%s", f, desc[i],
					i == 7 ? "" : ", ");
		}
#else
		p += sprintf(p, "%s: ", cnp->name);
		int i;
		p += sprintf(p, "%lu total, ", dt);
		for (i = 0; i != 8; i++) {
			unsigned long d = cnp->val[i] - cpp->val[i];
			p += sprintf(p, "%lu %s%s", d, desc[i],
					i == 7 ? "" : ", ");
		}
#endif
		cmd_printf(CLR_MESSAGE, "%s: %s\n", __func__, str);
		cpp = cpp->next;
		cnp = cnp->next;
	}
#endif
}

static void system_perf()
{
	data.schedule = time(NULL) + SYSTEM_INTERVAL;
	struct stat_t *stat = data.buf;
	if (!stat) {
		stat = malloc(sizeof(struct stat_t));
		stat->cpu = NULL;
	}
	system_perf_cpu(stat);

	// Triple-buffering for calculating differences
	pthread_mutex_lock(&data.lock);
	data.buf = data.prev;
	data.prev = data.now;
	data.now = stat;
	pthread_mutex_unlock(&data.lock);

	if (data.prev)
		system_perf_print();
}

void system_init()
{
	pthread_mutex_init(&data.lock, NULL);
	system_perf();
}

void system_deinit()
{
	pthread_mutex_lock(&data.lock);
	system_perf_free(data.buf);
	system_perf_free(data.prev);
	system_perf_free(data.now);
	data.buf = data.prev = data.now = NULL;
	pthread_mutex_unlock(&data.lock);
	pthread_mutex_destroy(&data.lock);
}

void system_tick(const char *str)
{
	// Wait until scheduled time passed
	if (time(NULL) - data.schedule >= 0)
		system_perf();
}

static struct json_object *system_json(struct json_object *act)
{
	struct json_object *obj = json_object_new_object();

	// boolean: detailed
	json_bool detailed = 0;
	struct json_object *detailedobj;
	if (json_object_object_get_ex(act, "detailed", &detailedobj))
		detailed = json_object_get_boolean(detailedobj);

	// CPU information may not be available during startup
	if (!data.prev)
		return obj;

	// CPU array
	struct json_object *cpus = json_object_new_array();
	pthread_mutex_lock(&data.lock);
	if (!data.prev) {
		pthread_mutex_unlock(&data.lock);
		return obj;
	}
	struct stat_cpu_t *cpp = data.prev->cpu;
	struct stat_cpu_t *cnp = data.now->cpu;
	while (cpp && cnp) {
		struct json_object *cpu = json_object_new_array();
		// name
		json_object_array_add(cpu, json_object_new_string(cpp->name));
		// total
		unsigned long dt = cnp->total - cpp->total;
		json_object_array_add(cpu, json_object_new_int64(dt));
		if (!detailed) {
			// idle
			unsigned long d = cnp->idle - cpp->idle;
			json_object_array_add(cpu, json_object_new_int64(d));
		} else {
			// user, nice, system, idle, iowait, irq, softirq, steal
			int i;
			for (i = 0; i != 8; i++) {
				unsigned long d = cnp->val[i] - cpp->val[i];
				json_object_array_add(cpu,
						json_object_new_int64(d));
			}
		}
		// Append to array
		json_object_array_add(cpus, cpu);
		cpp = cpp->next;
		cnp = cnp->next;
	}
	pthread_mutex_unlock(&data.lock);
	json_object_object_add(obj, "cpus", cpus);

	return obj;
}

struct mon_module_t mon_system = {
	.name = "system",
	.line = system_tick,
	.json = system_json,
};

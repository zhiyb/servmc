#include <stdio.h>
#include <string.h>
#include <time.h>
#include <json.h>
#include "net.h"
#include "cmd.h"
#include "exec.h"
#include "monitor.h"
#include "config.h"
#include "mon/restart.h"

static struct {
	char *version;
	int pending;
	time_t schedule;
} status = {NULL, 0, (time_t)-1};

static struct json_object *find(struct json_object *obj, const char *name)
{
	struct json_object_iterator it = json_object_iter_begin(obj);
	struct json_object_iterator ite = json_object_iter_end(obj);
	while (!json_object_iter_equal(&it, &ite)) {
		if (strcmp(name, json_object_iter_peek_name(&it)) == 0)
			return json_object_iter_peek_value(&it);
		json_object_iter_next(&it);
	}
	cmd_printf(CLR_ERROR, "%s: Cannot find %s in JSON\n", __func__, name);
	return NULL;
}

static struct json_object *update_parse(const char *doc, size_t size)
{
	struct json_tokener *tok = json_tokener_new();
	struct json_object *root = json_tokener_parse_ex(tok, doc, size);
	int err = json_tokener_get_error(tok);
	if (err != json_tokener_success)
		cmd_printf(CLR_ERROR, "%s: Error parsing json: %s\n",
				__func__, json_tokener_error_desc(err));
	json_tokener_free(tok);
	return root;
}

static const char *update_latest(struct json_object *root, const char *type)
{
	struct json_object *obj = root;
	if (!(obj = find(obj, "latest")))
		return NULL;
	if (!(obj = find(obj, type)))
		return NULL;
	return json_object_get_string(obj);
}

static struct json_object *update_version(
		struct json_object *root, const char *id)
{
	struct json_object *obj = root;
	if (!(obj = find(obj, "versions")))
		return NULL;

	struct json_object *vobj = NULL;
	int len = json_object_array_length(obj), i;
	for (i = 0; i != len; i++) {
		struct json_object *o = json_object_array_get_idx(obj, i);
		if (strcmp(id, json_object_get_string(find(o, "id"))) == 0) {
			vobj = o;
			break;
		}
	}

	return vobj;
}

static struct json_object *update_server(struct json_object *root)
{
	struct json_object *obj = root;
	if (!(obj = find(obj, "downloads")))
		return NULL;
	if (!(obj = find(obj, "server")))
		return NULL;
	return obj;
}

static const char *update_get_url(struct json_object *obj)
{
	return json_object_get_string(find(obj, "url"));
}

static const char *update_get_sha1(struct json_object *obj)
{
	return json_object_get_string(find(obj, "sha1"));
}

static void update_free(struct json_object *obj)
{
	json_object_put(obj);
}

void update()
{
	static const char *type = UPDATE_TYPE;
	static const char *path = SERVER_PATH;

	// Check version manifest
	cmd_printf(CLR_UPDATE, "%s: Downloading %s\n",
			__func__, MANIFEST_URL);
	size_t size;
	char *p = net_get(MANIFEST_URL, &size);

	struct json_object *root = update_parse(p, size);
	const char *ver = update_latest(root, type);
	struct json_object *obj = update_version(root, ver);
	const char *url = update_get_url(obj);
	cmd_printf(CLR_UPDATE, "%s: Latest %s version: %s\n",
			__func__, type, ver);

	if (status.version && strcmp(status.version, ver) == 0) {
		cmd_printf(CLR_UPDATE, "%s: No updates available\n", __func__);
		goto ret;
	}

	// Download new version
	cmd_printf(CLR_UPDATE, "%s: Downloading %s\n", __func__, url);
	free(p);
	p = net_get(url, &size);

	struct json_object *vobj = update_parse(p, size);
	obj = update_server(vobj);
	url = update_get_url(obj);
	{
		char file[strlen(path) + 1 + strlen(ver) + 4 + 1];
		sprintf(file, "%s/%s.jar", path, ver);
		const char *sha1 = update_get_sha1(obj);

		// Download new sever JAR
		if (net_download(url, file, sha1) == 0) {
			status.pending = !!status.version;
			status.version = realloc(status.version,
					strlen(ver) + 1);
			strcpy(status.version, ver);
			restart_schedule();
		}

	}

	update_free(vobj);
ret:	update_free(root);
	free(p);
	status.schedule = time(NULL) + UPDATE_INTERVAL;
}

void update_tick()
{
	if (status.schedule == (time_t)-1)
		return;
	// Wait until scheduled time passed
	if (time(NULL) - status.schedule < 0)
		return;
	// It is not safe to start update during server start up
	if (exec_status() >= 0 && !monitor_server_status())
		return;
	update();
}

const char *update_current()
{
	return status.version;
}

int update_pending(int clear)
{
	int v = status.pending;
	if (clear)
		status.pending = 0;
	return v;
}

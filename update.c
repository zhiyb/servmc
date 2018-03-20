#include <stdio.h>
#include <string.h>
#include <json.h>
#include "net.h"

#define MFURL	"https://launchermeta.mojang.com/mc/game/version_manifest.json"

static struct json_object *find(struct json_object *obj, const char *name)
{
	struct json_object_iterator it = json_object_iter_begin(obj);
	struct json_object_iterator ite = json_object_iter_end(obj);
	while (!json_object_iter_equal(&it, &ite)) {
		if (strcmp(name, json_object_iter_peek_name(&it)) == 0)
			return json_object_iter_peek_value(&it);
		json_object_iter_next(&it);
	}
	fprintf(stderr, "%s: Cannot find %s in JSON\n", __func__, name);
	return NULL;
}

static struct json_object *update_parse(const char *doc, size_t size)
{
	struct json_tokener *tok = json_tokener_new();
	struct json_object *root = json_tokener_parse_ex(tok, doc, size);

	int err = json_tokener_get_error(tok);
	if (err != json_tokener_success) {
		fprintf(stderr, "%s: Error parsing json: %s\n",
				__func__, json_tokener_error_desc(err));
		goto ret;
	}

ret:	json_tokener_free(tok);
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
	static const char *type = "snapshot";
	static const char *path = "server";

	// Check version manifest
	fprintf(stderr, "%s: Downloading %s\n", __func__, MFURL);
	size_t size;
	char *p = net_get(MFURL, &size);

	struct json_object *root = update_parse(p, size);
	const char *ver = update_latest(root, type);;
	struct json_object *obj = update_version(root, ver);
	const char *url = update_get_url(obj);
	fprintf(stderr, "%s: Latest %s version: %s\n", __func__, type, ver);

	// Download new version
	free(p);
	fprintf(stderr, "%s: Downloading %s\n", __func__, url);
	p = net_get(url, &size);

	struct json_object *vobj = update_parse(p, size);
	obj = update_server(vobj);
	url = update_get_url(obj);
	const char *sha1 = update_get_sha1(obj);
	fprintf(stderr, "%s: Downloading %s to %s/%s.jar (sha1sum: %s)\n",
			__func__, url, path, ver, sha1);

	free(p);
	update_free(vobj);
	update_free(root);
}

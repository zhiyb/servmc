#include <string.h>
#include <json.h>
#include "cmd.h"
#include "update.h"
#include "config.h"

static struct json_object *query_parse(const char *doc)
{
	struct json_tokener *tok = json_tokener_new();
	struct json_object *obj = json_tokener_parse_ex(tok, doc, strlen(doc));
	int err = json_tokener_get_error(tok);
	if (err != json_tokener_success)
		cmd_printf(CLR_ERROR, "%s: Error parsing json: %s\n",
				__func__, json_tokener_error_desc(err));
	json_tokener_free(tok);
	return obj;
}

static const char *query_stringify(struct json_object *obj)
{
	return json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PLAIN);
}

struct json_object *query_set(struct json_object *qobj, const char *type)
{
	return NULL;
}

struct json_object *query_get(struct json_object *qobj, const char *type)
{
	if (strcmp(type, "version") == 0)
		return json_object_new_string(update_current());
	return NULL;
}

struct json_object *query_json(struct json_object *qobj)
{
	if (!json_object_is_type(qobj, json_type_object))
		return NULL;
	int op_set = 0;
	struct json_object *set;
	if (json_object_object_get_ex(qobj, "set", &set) &&
			json_object_get_boolean(set))
		op_set = 1;
	const char *op_type = NULL;
	struct json_object *type;
	if (json_object_object_get_ex(qobj, "type", &type))
		op_type = json_object_get_string(type);
	if (!op_type)
		return NULL;
	if (op_set)
		return query_set(qobj, op_type);
	return query_get(qobj, op_type);
}

char *query_json_doc(const char *doc, const size_t prefix)
{
	struct json_object *qobj = query_parse(doc);
	struct json_object *obj = query_json(qobj);
	json_object_put(qobj);

	const char *sp = query_stringify(obj);
	size_t len = strlen(sp);
	char *str = malloc(prefix + len + 1);
	strcpy(str + prefix, sp);
	json_object_put(obj);
	return str;
}

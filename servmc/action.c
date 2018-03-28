#include <string.h>
#include <json.h>
#include "cmd.h"
#include "update.h"
#include "monitor.h"
#include "config.h"

static struct json_object *action_parse(const char *doc)
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

static const char *action_stringify(struct json_object *obj)
{
	return json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PLAIN);
}

struct json_object *action_query(struct json_object *aobj)
{
	const char *op_type = NULL;
	struct json_object *type;
	if (!json_object_object_get_ex(aobj, "type", &type) ||
		!(op_type = json_object_get_string(type)))
		return NULL;

	if (strcmp(op_type, "version") == 0)
		return json_object_new_string(update_current());
	else
		return mon_json(aobj, op_type);
	return NULL;
}

struct json_object *action_json(struct json_object *aobj)
{
	if (!json_object_is_type(aobj, json_type_object))
		return NULL;

	const char *op_act = NULL;
	struct json_object *act;
	if (!json_object_object_get_ex(aobj, "action", &act) ||
		!(op_act = json_object_get_string(act)))
		return NULL;

	if (strcmp(op_act, "query") == 0)
		return action_query(aobj);
	return NULL;
}

char *action_json_doc(const char *doc, const size_t prefix)
{
	struct json_object *aobj = action_parse(doc);
	struct json_object *obj = action_json(aobj);
	json_object_put(aobj);

	const char *sp = action_stringify(obj);
	size_t len = strlen(sp);
	char *str = malloc(prefix + len + 1);
	strcpy(str + prefix, sp);
	json_object_put(obj);
	return str;
}

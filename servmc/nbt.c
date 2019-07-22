#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "nbt.h"

struct __attribute__((packed)) nbt_t {
	uint8_t id;
	uint16_t len;
	char name[];
};

struct nbt_value_t {
	uint8_t type;
	char *tag;
	union nbt_data_t {
		int8_t i8;
		int16_t i16;
		int32_t i32;
		int64_t i64;
		float f;
		double d;
		char *str;
		struct nbt_array_t {
			uint8_t type;
			int32_t len;
			void *ptr;
		} array;
		struct nbt_list_t {
			struct nbt_list_t *next;
			struct nbt_value_t *val;
		} *list;
		uint8_t raw[0];
	} v[0];
};

static const char * const tag_names[] = {
	"TAG_End", "TAG_Byte", "TAG_Short", "TAG_Int", "TAG_Long",
	"TAG_Float", "TAG_Double", "TAG_Byte_Array", "TAG_String",
	"TAG_List", "TAG_Compound", "TAG_Int_Array", "TAG_Long_Array",
};

static const int tag_sizes[] = {
	0, 1, 2, 4, 8, 4, 8, sizeof(struct nbt_array_t), sizeof(char *),
	sizeof(struct nbt_array_t), sizeof(struct nbt_list_t *),
	sizeof(struct nbt_array_t), sizeof(struct nbt_array_t),
};

static struct nbt_value_t *nbt_parse_value(void **p, size_t *size);
static void nbt_dump_value(int level, struct nbt_value_t *val);

struct nbt_value_t *nbt_new(uint8_t type, const char *tag, size_t len)
{
	struct nbt_value_t *nbt = calloc(1, sizeof(struct nbt_value_t) +
			tag_sizes[type]);
	nbt->type = type;
	if (tag) {
		if (!len)
			len = strlen(tag);
		nbt->tag = calloc(len + 1, 1);
		memcpy(nbt->tag, tag, len);
	}
	return nbt;
}

static void nbt_free_data(uint8_t type, union nbt_data_t *p)
{
	if (type == TAG_Byte_Array || type == TAG_List ||
			type == TAG_Int_Array || type == TAG_Long_Array) {
		size_t size = tag_sizes[p->array.type];
		int32_t len = p->array.len;
		void *ptr = p->array.ptr;
		while (len--) {
			nbt_free_data(p->array.type, ptr);
			ptr += size;
		}
		free(p->array.ptr);
	} else if (type == TAG_String) {
		free(p->str);
	} else if (type == TAG_Compound) {
		struct nbt_list_t *lp = p->list;
		while (lp) {
			struct nbt_list_t *p = lp;
			lp = lp->next;
			nbt_free(p->val);
			free(p);
		}
	}
}

void nbt_free(struct nbt_value_t *nbt)
{
	if (!nbt)
		return;
	nbt_free_data(nbt->type, nbt->v);
	free(nbt->tag);
	free(nbt);
}

int nbt_set_int(struct nbt_value_t *val, int64_t v)
{
	uint8_t type = val->type;
	if (type != TAG_Byte && type != TAG_Short &&
			type != TAG_Int && type != TAG_Long)
		return -1;
	memcpy(val->v[0].raw, &v, tag_sizes[val->type]);
	return 0;
}

int nbt_set_float(struct nbt_value_t *val, float v)
{
	if (val->type != TAG_Float)
		return -1;
	val->v[0].f = v;
	return 0;
}

int nbt_set_double(struct nbt_value_t *val, double v)
{
	if (val->type != TAG_Double)
		return -1;
	val->v[0].d = v;
	return 0;
}

int nbt_set_string(struct nbt_value_t *val, const char *str, size_t len)
{
	if (val->type != TAG_String)
		return -1;
	if (!len)
		len = strlen(str);
	free(val->v[0].str);
	if (str) {
		char *sp = calloc(len + 1, 1);
		memcpy(sp, str, len);
		val->v[0].str = sp;
	} else {
		val->v[0].str = NULL;
	}
	return 0;
}

static int nbt_data_set_array(union nbt_data_t *val, uint8_t atype, int32_t len)
{
	if (len < 0)
		len = 0;
	free(val->array.ptr);
	val->array.type = atype;
	val->array.len = len;
	val->array.ptr = calloc(len, tag_sizes[atype]);
	return 0;
}

int nbt_set_array(struct nbt_value_t *val, uint8_t atype, int32_t len)
{
	if (val->type == TAG_Byte_Array && atype != TAG_Byte)
		return -1;
	if (val->type == TAG_Int_Array && atype != TAG_Int)
		return -1;
	if (val->type == TAG_Long_Array && atype != TAG_Long)
		return -1;
	return nbt_data_set_array(val->v, atype, len);
}

int nbt_array_add(struct nbt_value_t *array, struct nbt_value_t *val)
{
	if (array->v[0].array.type != val->type)
		return -1;
	void **pp = &array->v[0].array.ptr;
	int size = tag_sizes[array->v[0].array.type];
	int32_t len = array->v[0].array.len;
	*pp = realloc(*pp, size * (len + 1));
	memcpy(*pp + size * len, val->v[0].raw, size);
	array->v[0].array.len = len + 1;
	free(val);
	return 0;
}

static void nbt_data_list_add(union nbt_data_t *list, struct nbt_value_t *val)
{
	struct nbt_list_t *p = malloc(sizeof(struct nbt_list_t));
	p->next = list->list;
	p->val = val;
	list->list = p;
}

void nbt_list_add(struct nbt_value_t *list, struct nbt_value_t *val)
{
	nbt_data_list_add(list->v, val);
}

static uint64_t nbt_read_value(void *p, int size)
{
	uint64_t v = 0;
	while (size--) {
		v <<= 8;
		v |= *(uint8_t *)p++;
	}
	return v;
}

static int nbt_parse_data(uint8_t type, union nbt_data_t *dp,
		void **p, size_t *size)
{
	if (type == TAG_End) {
		fprintf(stderr, "%s: Unexpected TAG_End\n", __func__);
		return -1;
	} else if (type == TAG_Byte || type == TAG_Short ||
			type == TAG_Int || type == TAG_Long) {
		int vs = tag_sizes[type];
		int64_t v = nbt_read_value(*p, vs);
		memcpy(dp->raw, &v, vs);
		*p += vs;
		*size -= vs;
	} else if (type == TAG_Float) {
		union {
			uint32_t u;
			float f;
		} f;
		f.u = nbt_read_value(*p, 4);
		dp->f = f.f;
		*p += 4;
		*size -= 4;
	} else if (type == TAG_Double) {
		union {
			uint64_t u;
			double d;
		} d;
		d.u = nbt_read_value(*p, 8);
		dp->d = d.d;
		*p += 8;
		*size -= 8;
	} else if (type == TAG_String) {
		// String length
		int16_t len = nbt_read_value(*p, 2);
		*p += 2;
		*size -= 2;
		// String data
		char *str = calloc(len + 1, 1);
		memcpy(str, *p, len);
		dp->str = str;
		*p += len;
		*size -= len;
	} else if (type == TAG_Compound) {
		while (((struct nbt_t *)*p)->id != TAG_End) {
			struct nbt_value_t *val = nbt_parse_value(p, size);
			if (!val)
				return -1;
			nbt_data_list_add(dp, val);
		}
		*p += 1;
		*size -= 1;
	} else if (type == TAG_Byte_Array || type == TAG_List ||
			type == TAG_Int_Array || type == TAG_Long_Array) {
		uint8_t atype = 0;
		if (type == TAG_Byte_Array)
			atype = TAG_Byte;
		else if (type == TAG_List) {
			atype = *(char *)(*p);
			*p += 1;
			*size -= 1;
		} else if (type == TAG_Int_Array)
			atype = TAG_Int;
		else if (type == TAG_Long_Array)
			atype = TAG_Long;
		// Array length
		int32_t len = nbt_read_value(*p, 4);
		if (len < 0)
			len = 0;
		*p += 4;
		*size -= 4;
		// Array data
		nbt_data_set_array(dp, atype, len);
		void *vp = dp->array.ptr;
		while (len--) {
			if (nbt_parse_data(atype, vp, p, size))
				return -1;
			vp += tag_sizes[atype];
		}
	} else {
		fprintf(stderr, "%s: Unknown type: %u\n", __func__, type);
		return -1;
	}
	return 0;
}

static struct nbt_value_t *nbt_parse_value(void **p, size_t *size)
{
	struct nbt_t *nbt = *p;
	// Parse TAG type
	uint8_t type = nbt->id;
	if (type == TAG_End) {
		fprintf(stderr, "%s: Unexpected TAG_End\n", __func__);
		return NULL;
	}
	// Parse TAG name
	uint16_t len = nbt_read_value(&nbt->len, 2);
	// Parse TAG data
	struct nbt_value_t *val = nbt_new(type, nbt->name, len);
	*p += 3 + len;
	*size -= 3 + len;
	if (nbt_parse_data(type, val->v, p, size)) {
		nbt_free(val);
		val = NULL;
	}
	return val;
}

struct nbt_value_t *nbt_parse(void *p, size_t size)
{
	return nbt_parse_value(&p, &size);
}

static void nbt_dump_level(int level)
{
	while (level--)
		fputs("  ",  stdout);
}

static void nbt_dump_data(int level, uint8_t type, union nbt_data_t *p)
{
	if (type == TAG_End) {
		fflush(stdout);
		fprintf(stderr, "%s: Unexpected TAG_End\n", __func__);
	} else if (type == TAG_Byte) {
		fprintf(stdout, "%i", (int)p->i8);
	} else if (type == TAG_Short) {
		fprintf(stdout, "%i", (int)p->i16);
	} else if (type == TAG_Int) {
		fprintf(stdout, "%i", p->i32);
	} else if (type == TAG_Long) {
		fprintf(stdout, "%liL", p->i64);
	} else if (type == TAG_Float) {
		fprintf(stdout, "%g", p->f);
	} else if (type == TAG_Double) {
		fprintf(stdout, "%g", p->d);
	} else if (type == TAG_String) {
		if (p->str)
			fprintf(stdout, "'%s'", p->str);
		else
			fprintf(stdout, "null");
	} else if (type == TAG_Compound) {
		fputs(p->list ? "{\n" : "{}", stdout);
		level++;
		struct nbt_list_t *lp = p->list;
		while (lp) {
			nbt_dump_value(level, lp->val);
			lp = lp->next;
		}
		if (p->list) {
			nbt_dump_level(--level);
			fputc('}', stdout);
		}
	} else if (type == TAG_Byte_Array || type == TAG_List ||
			type == TAG_Int_Array || type == TAG_Long_Array) {
		uint8_t type = p->array.type;
		// Array length
		int32_t len = p->array.len;
		fprintf(stdout, "%s %i entries {%c", tag_names[type],
				len, len ? '\n' : '}');
		// Array data
		if (len)
			nbt_dump_level(++level);
		int size = tag_sizes[type];
		void *dp = p->array.ptr;
		while (len--) {
			nbt_dump_data(level, type, dp);
			dp += size;
			if (len)
				fputs(", ", stdout);
		}
		if (p->array.len) {
			fputc('\n', stdout);
			nbt_dump_level(--level);
			fputc('}', stdout);
		}
	} else {
		fflush(stdout);
		fprintf(stderr, "%s: Unknown type: %u\n", __func__, type);
	}
}

static void nbt_dump_value(int level, struct nbt_value_t *val)
{
	nbt_dump_level(level);
	fprintf(stdout, "%s('%s'): ", tag_names[val->type], val->tag);
	nbt_dump_data(level, val->type, val->v);
	fputc('\n', stdout);
}

void nbt_dump(struct nbt_value_t *val)
{
	if (!val)
		return;
	nbt_dump_value(0, val);
}

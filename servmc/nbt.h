#pragma once

#include <stdint.h>

enum TAG_Types {TAG_End = 0, TAG_Byte, TAG_Short, TAG_Int, TAG_Long,
	TAG_Float, TAG_Double, TAG_Byte_Array, TAG_String,
	TAG_List, TAG_Compound, TAG_Int_Array, TAG_Long_Array};

struct nbt_value_t;

struct nbt_value_t *nbt_new(uint8_t type, const char *tag, size_t len);
void nbt_free(struct nbt_value_t *nbt);
int nbt_set_int(struct nbt_value_t *val, int64_t v);
int nbt_set_float(struct nbt_value_t *val, float v);
int nbt_set_double(struct nbt_value_t *val, double v);
int nbt_set_string(struct nbt_value_t *val, const char *str, size_t len);
int nbt_set_array(struct nbt_value_t *val, uint8_t atype, int32_t len);

int nbt_array_add(struct nbt_value_t *array, struct nbt_value_t *val);
void nbt_list_add(struct nbt_value_t *list, struct nbt_value_t *val);

struct nbt_value_t *nbt_parse(void *p, size_t size);
void nbt_dump(struct nbt_value_t *val);

#pragma once

void net_init();
int net_status();
char *net_get(const char *url, size_t *size);

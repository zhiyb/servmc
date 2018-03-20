#pragma once

void net_init();
int net_status();
char *net_get(const char *url, size_t *size);
int net_download(const char *url, const char *file, const char *sha1);

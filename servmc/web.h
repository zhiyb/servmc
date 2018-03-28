#pragma once

void web_init();
void web_quit();
void web_message_f(const char *colour, const char *fmt, ...);
void web_message(const char *colour, size_t size, const char *fmt, va_list ap);

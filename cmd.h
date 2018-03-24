#pragma once

void cmd_init();
int cmd_rfd();
void cmd_process();
void cmd_quit();
int cmd_shutdown();
void cmd_external(const char *colour);
int cmd_printf(const char *colour, const char *fmt, ...);

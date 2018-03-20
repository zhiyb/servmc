#pragma once

int exec_server(const char *jar);
void exec_quit();
void exec_write_stdin(const char *str);
int exec_stdout_rfd();
int exec_stdout_process();
void *exec_stdout_loop(void *);

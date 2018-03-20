#pragma once

int exec_server(const char *jar);
void exec_quit();
void exec_write_stdin(const char *str);
int exec_status();
int exec_rfd(int err);
int exec_process(int err);
void *exec_stdout_loop(void *);

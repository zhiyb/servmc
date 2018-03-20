#pragma once

int exec_server(const char *dir, const char *jar);
void exec_quit();

int exec_status();
void exec_write_stdin(const char *str);
int exec_rfd(int err);
int exec_process(int err);

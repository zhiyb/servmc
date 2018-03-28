#pragma once

int exec_server(const char *dir, const char *jar);
void exec_stop();
void exec_kill();
void exec_quit();

int exec_backup();

int exec_status();
void exec_write_stdin(const char *prompt, const char *str, int echo);
int exec_rfd(int err);
int exec_process(int err);

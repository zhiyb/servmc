#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include "monitor.h"

#define READ	0
#define WRITE	1

static pid_t pid = -1;
static int fdin[2], fdout[2], fderr[2];
static FILE *fin, *fout, *ferr;

int exec_status()
{
	return pid;
}

void exec_write_stdin(const char *str)
{
	if (pid < 0)
		return;
	fputs(str, fin);
	fputc('\n', fin);
	fflush(fin);
}

int exec_rfd(int err)
{
	if (pid < 0)
		return -1;
	return fileno(err ? ferr : fout);
}

static char *exec_read(int err)
{
	char *s = NULL;
	unsigned int len = 0;
	do {
		char buf[256];
		if (!fgets(buf, sizeof(buf), err ? ferr : fout))
			return s;
		unsigned int slen = strlen(buf);
		s = realloc(s, len + slen + 1);
		memcpy(s + len, buf, slen + 1);
		len += slen;
	} while (s[len - 1] != '\n');
	return s;
}

int exec_process(int err)
{
	char *str = exec_read(err);
	if (!str)
		return EOF;
	fputs(str, stdout);
	monitor_line(str);
	free(str);
	return 0;
}

int exec_server(const char *dir, const char *jar)
{
	if (pid >= 0)
		return EBUSY;
	fprintf(stderr, "%s: Starting %s in %s\n", __func__, jar, dir);
	monitor_server_start();
	// Create input/output pipes
	pipe(fdin);
	pipe(fdout);
	pipe(fderr);

	// Create child process
	pid = fork();
	if (pid < 0) {
		monitor_server_stop();
		return pid;
	}

	if (pid) {
		// Parent process
		close(fdout[WRITE]);
		close(fderr[WRITE]);
		fin = fdopen(fdin[WRITE], "w");
		fout = fdopen(fdout[READ], "r");
		ferr = fdopen(fderr[READ], "r");
		setlinebuf(fin);
		setbuf(fout, NULL);
		setbuf(ferr, NULL);
		return 0;
	}

	// Child process
	// Redirect stdio
	dup2(fdin[READ], STDIN_FILENO);
	dup2(fdout[WRITE], STDOUT_FILENO);
	dup2(fderr[WRITE], STDERR_FILENO);

	// Execute
	if (dir) {
		int err = chdir(dir);
		if (err) {
			fprintf(stderr, "%s: Error changing directory: %s\n",
					__func__, strerror(errno));
			exit(0);
		}
	}
	execlp("java", "java", "-jar", jar, "nogui", (char *)NULL);
	return 0;
}

void exec_quit()
{
	monitor_server_stop();
	if (pid < 0)
		return;
	fprintf(stderr, "%s: Process exit\n", __func__);
	wait(NULL);
	pid = -1;
	fclose(fin);
	close(fdin[READ]);
	fclose(fout);
	fclose(ferr);
}

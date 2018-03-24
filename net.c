#include <errno.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <curl/curl.h>
#include "cmd.h"
#include "config.h"

struct net_data_t {
	curl_off_t size;
	void *p;
};

static CURLcode fail = CURLE_OK;

void net_init()
{
	fail = curl_global_init(CURL_GLOBAL_ALL);
}

int net_status()
{
	return fail;
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	return size * nmemb;
}

char *net_get(const char *url, size_t *size)
{
	if (net_status())
		return NULL;

	CURL *curl = curl_easy_init();
	if (!curl)
		return NULL;

	FILE *fp = tmpfile();
	if (!fp) {
		cmd_printf(CLR_ERROR, "%s: Temporary file error: %s\n",
				__func__, strerror(errno));
		return NULL;
	}

	void *p = NULL;
	CURLcode res;
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
	res = curl_easy_perform(curl);

	if (res != CURLE_OK) {
		cmd_printf(CLR_ERROR, "%s: libcurl error: %s\n",
				__func__, curl_easy_strerror(res));
		goto ret;
	}

	double ds;
	curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &ds);
	*size = ds;

	rewind(fp);
	p = malloc(*size);
	fread(p, *size, 1, fp);

ret:
	curl_easy_cleanup(curl);
	fclose(fp);
	return p;
}

int net_download(const char *url, const char *file, const char *sha1)
{
	// Check file existence and size
	struct stat st;
	if (stat(file, &st) != 0)
		goto dl;
#if 0
	// Check file integrity
	int fd = open(file, O_RDONLY);
	if (fd < 0)
		goto dl;
	void *buf = malloc(st.st_size);
	if (read(fd, buf, st.st_size) == st.st_size) {
		// TODO
	}
	free(buf);
	close(fd);
#endif
	cmd_printf(CLR_MESSAGE, "%s: File %s exists, skip\n", __func__, file);
	return 0;

dl:	cmd_printf(CLR_MESSAGE, "%s: Downloading %s to %s (sha1sum: %s)\n",
			__func__, url, file, sha1);

	if (net_status())
		return ENODEV;

	CURL *curl = curl_easy_init();
	if (!curl)
		return ENOMEM;

	char path[strlen(file) + 1];
	strcpy(path, file);
	const char *dir = dirname(path);
	int err = mkdir(dir, 0755);
	if (err && errno != EEXIST) {
		cmd_printf(CLR_ERROR, "%s: Cannot create directory %s: %s\n",
				__func__, dir, strerror(errno));
		return errno;
	}

	FILE *fp = fopen(file, "wb");
	if (!fp) {
		cmd_printf(CLR_ERROR, "%s: Cannot open file for write: %s\n",
				__func__, strerror(errno));
		return errno;
	}

	CURLcode res;
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	fclose(fp);

	if (res != CURLE_OK) {
		cmd_printf(CLR_ERROR, "%s: libcurl error: %s\n",
				__func__, curl_easy_strerror(res));
		return EIO;
	}
	return 0;
}

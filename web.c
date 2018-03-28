#include <magic.h>
#include <pthread.h>
#include <libwebsockets.h>
#include "cmd.h"
#include "query.h"
#include "config.h"

#define RX_BUFFER_SIZE	1024

struct post_t {
	char *p, *rp;
	size_t len, rlen;
};

static pthread_t thread;
static pthread_mutex_t msgs_mutex;
static struct lws_context *lws_ctx = NULL;
static struct lws * volatile client = NULL;
static magic_t magic = NULL;
static char *http_path = NULL;
static int quit = 0;
static struct message_t {
	struct message_t * volatile next;
	const char *colour;
	char *msg;
	size_t size;
} * volatile msgs;

static int web_http(struct lws *wsi, enum lws_callback_reasons reason,
		void *user, void *in, size_t len);
static int web_console(struct lws *wsi, enum lws_callback_reasons reason,
		void *user, void *in, size_t len);

static struct lws_protocols protocols[] = {
	// The first protocol must always be the HTTP handler
	// Name, callback, per session data, max frame size / rx buffer
	{"http-only", web_http, sizeof(struct post_t), RX_BUFFER_SIZE},
	{"web-console", web_console, 0, RX_BUFFER_SIZE},
	{NULL, NULL, 0, 0}	// Terminator
};

static const char *web_file_ext(const char *path)
{
	const char *dot = strrchr(path, '.');
	if (!dot || dot == path)
		return "";
	return dot + 1;
}

static int web_post_url(struct lws *wsi)
{
	size_t size = lws_hdr_total_length(wsi, WSI_TOKEN_POST_URI);
	if (size > 32)
		return -1;
	char url[size + 1];
	int n = lws_hdr_copy(wsi, url, sizeof(url), WSI_TOKEN_POST_URI);
	if (n < 0)
		return -1;
	cmd_printf(CLR_WEB, "%s: HTTP POST: %s\n", __func__, url);
	if (strcmp(url, "/q") != 0)
		return -1;
	return 0;
}

static int web_http(struct lws *wsi, enum lws_callback_reasons reason,
		void *user, void *in, size_t len)
{
	struct post_t *psd = (struct post_t *)user;
	static const size_t flen = strlen(WEB_PATH);
	if (reason == LWS_CALLBACK_HTTP_BODY) {
		psd->p = realloc(psd->p, psd->len + len + 1);
		memcpy(psd->p + psd->len, in, len);
		psd->len += len;
		*(psd->p + psd->len) = 0;
	} else if (reason == LWS_CALLBACK_HTTP_BODY_COMPLETION) {
		cmd_printf(CLR_WEB, "%s: HTTP POST data: %s\n",
				__func__, psd->p);
		// Process POST data
		psd->rp = query_json_doc(psd->p, LWS_PRE);
		psd->rlen = strlen(psd->rp + LWS_PRE);
		// Write response header
		unsigned char *hdr = malloc(LWS_PRE + 1024);
		unsigned char *start = hdr + LWS_PRE, *end = start + 1024;
		unsigned char *p = (void *)start;
		if (lws_add_http_header_status(wsi, HTTP_STATUS_OK, &p, end) ||
		lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE,
			(const void *)"application/json", 16, &p, end) ||
		lws_add_http_header_content_length(wsi, psd->rlen, &p, end) ||
		lws_finalize_http_header(wsi, &p, end) ||
		lws_write(wsi, start, p - start, LWS_WRITE_HTTP_HEADERS) < 0) {
			free(hdr);
			return -1;
		}
		free(hdr);
		lws_callback_on_writable(wsi);
	} else if (reason == LWS_CALLBACK_HTTP_WRITEABLE) {
		if (!psd->len)
			return -1;
		if (lws_write(wsi, (unsigned char *)psd->rp + LWS_PRE,
			      psd->rlen, LWS_WRITE_HTTP) < 0)
			return -1;
		if (lws_http_transaction_completed(wsi))
			return -1;
	} else if (reason == LWS_CALLBACK_CLOSED_HTTP) {
		if (psd) {
			free(psd->p);
			psd->p = NULL;
			psd->len = 0;
			free(psd->rp);
			psd->rp = NULL;
			psd->rlen = 0;
		}
	} else if (reason == LWS_CALLBACK_HTTP) {
		if (lws_hdr_total_length(wsi, WSI_TOKEN_POST_URI))
			return web_post_url(wsi);
		const char *url = in;
		http_path = realloc(http_path, flen + len + 2 + 10);
		// TODO: URL validation
		sprintf(http_path, WEB_PATH "%s%s", url,
				url[len - 1] == '/' ? "index.html" : "");
		// Check file MIME type
		const char *ext = web_file_ext(http_path);
		const char *type = NULL;
		if (strcmp(ext, "html") == 0)
			type = "text/html";
		else if (strcmp(ext, "js") == 0)
			type = "application/javascript";
		else if (strcmp(ext, "css") == 0)
			type = "text/css";
		else
			type = magic_file(magic, http_path);
		// Send file
		cmd_printf(CLR_WEB, "%s: HTTP request: %s (%s: %s)\n",
				__func__, url, type, http_path);
		if (lws_serve_http_file(wsi, http_path, type, NULL, 0) < 0)
			return -1;
	} else if (reason == LWS_CALLBACK_RECEIVE) {
		cmd_printf(CLR_WEB, "%s:%u\n", __func__, __LINE__);
	} else if (reason == LWS_CALLBACK_HTTP_FILE_COMPLETION) {
		if (lws_http_transaction_completed(wsi))
			return -1;
	}
	return 0;
}

static void web_write_message(struct lws *wsi)
{
	if (!msgs)
		return;

	// Detach the oldest message from the queue
	pthread_mutex_lock(&msgs_mutex);
	struct message_t *mp = msgs;
	msgs = msgs->next;
	pthread_mutex_unlock(&msgs_mutex);

	lws_write(wsi, (void *)(mp->msg + LWS_PRE), mp->size, LWS_WRITE_TEXT);
	free(mp->msg);
	free(mp);

	if (msgs)
		lws_callback_on_writable(wsi);
}

static int web_console(struct lws *wsi, enum lws_callback_reasons reason,
		void *user, void *in, size_t len)
{
	switch (reason) {
	case LWS_CALLBACK_ESTABLISHED:
		if (client)
			lws_callback_on_writable(client);
		client = wsi;
		cmd_printf(CLR_WEB, "%s: WS %p connected\n", __func__, wsi);
		break;
	case LWS_CALLBACK_CLOSED:
		if (client == wsi)
			client = NULL;
		cmd_printf(CLR_WEB, "%s: WS %p disconnected\n", __func__, wsi);
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		if (client != wsi)
			return -1;
		web_write_message(wsi);
		break;
	case LWS_CALLBACK_RECEIVE:
		if (client != wsi)
			return -1;
		user = malloc(len + 1);
		memcpy(user, in, len);
		*((char *)user + len) = 0;
		cmd_line(user, 1);
		break;
	default:
		break;
	}
	return 0;
}

static void *web_thread(void *args)
{
	while (!quit) {
		lws_service(lws_ctx, 200);
		if (msgs && client)
			lws_callback_on_writable(client);
	}
	return NULL;
}

static void web_log(int level, const char *line)
{
	cmd_printf(CLR_LIBWS, "[WEB/%d] %s", level, line);
}

void web_init()
{
	magic = magic_open(MAGIC_MIME_TYPE);
	magic_load(magic, NULL);
	magic_compile(magic, NULL);

	struct lws_context_creation_info info = {
		.port = WEB_PORT,
		.protocols = protocols,
		.gid = -1, .uid = -1,
	};
	lws_set_log_level(7, web_log);
	lws_ctx = lws_create_context(&info);

	quit = 0;
	pthread_mutex_init(&msgs_mutex, NULL);
	pthread_create(&thread, NULL, web_thread, NULL);
}

void web_quit()
{
	quit = 1;
	lws_cancel_service(lws_ctx);
	pthread_join(thread, NULL);

	lws_context_destroy(lws_ctx);
	lws_ctx = NULL;
	client = NULL;
	pthread_mutex_destroy(&msgs_mutex);

	magic_close(magic);
	free(http_path);
	http_path = NULL;
}

static void web_enqueue(struct message_t *m)
{
	// Append to message list
	pthread_mutex_lock(&msgs_mutex);
	struct message_t * volatile *mp = &msgs;
	for (; *mp; mp = &(*mp)->next);
	*mp = m;
	pthread_mutex_unlock(&msgs_mutex);
}

void web_message(const char *colour, size_t size, const char *fmt, va_list ap)
{
	if (!client)
		return;
	struct message_t *m = malloc(sizeof(struct message_t));
	m->next = NULL;
	m->colour = colour;
	size_t clen = strlen(colour);
	m->msg = malloc(size + LWS_PRE + clen);
	strcpy(m->msg + LWS_PRE, colour);
	m->size = vsnprintf(m->msg + LWS_PRE + clen, size, fmt, ap) + clen + 1;
	web_enqueue(m);
}

void web_message_f(const char *colour, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int ret = vsnprintf(NULL, 0, fmt, args);
	va_end(args);
	va_start(args, fmt);
	web_message(colour, ret + 1, fmt, args);
	va_end(args);
}

#include <pthread.h>
#include <libwebsockets.h>
#include "cmd.h"
#include "config.h"

#define RX_BUFFER_SIZE	256

static pthread_t thread;
static pthread_mutex_t msgs_mutex;
static struct lws_context *lws_ctx = NULL;
static struct lws * volatile client = NULL;
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
	{"http-only", web_http, 0, 0},
	{"web-console", web_console, 0, RX_BUFFER_SIZE},
	{NULL, NULL, 0, 0}	// Terminator
};

static int web_http(struct lws *wsi, enum lws_callback_reasons reason,
		void *user, void *in, size_t len)
{
	static const size_t flen = strlen(WEB_PATH);
	if (reason == LWS_CALLBACK_HTTP) {
		char *url = in;
		char path[flen + len + 2 + 10];
		// TODO: URL validation
		snprintf(path, sizeof(path), WEB_PATH "%s%s", url,
				url[len - 1] == '/' ? "index.html" : "");
		cmd_printf(CLR_WEB, "%s: HTTP request: %s (%s)\n",
				__func__, url, path);
		lws_serve_http_file(wsi, path, "text/html", NULL, 0);
		return 1;
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

#include <pthread.h>
#include <libwebsockets.h>
#include "cmd.h"
#include "config.h"

#define RX_BUFFER_SIZE	256

static pthread_t thread;
static struct lws_context *lws_ctx = NULL;
static int quit = 0;

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
	if (reason == LWS_CALLBACK_HTTP)
		lws_serve_http_file(wsi, "web.html", "text/html", NULL, 0);
	return 0;
}

static int web_console(struct lws *wsi, enum lws_callback_reasons reason,
		void *user, void *in, size_t len)
{
	if (reason == LWS_CALLBACK_RECEIVE) {
	} else if (reason == LWS_CALLBACK_SERVER_WRITEABLE) {
	}
	return 0;
}

static void *web_thread(void *args)
{
	while (!quit)
		lws_service(lws_ctx, 1000);
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
	pthread_create(&thread, NULL, web_thread, NULL);
}

void web_quit()
{
	quit = 1;
	pthread_join(thread, NULL);
	lws_context_destroy(lws_ctx);
	lws_ctx = NULL;
}

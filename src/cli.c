
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <libwebsockets.h>

#include "gun.h"

static volatile int running = 1;

static int wsi_callback(struct lws *wsi, enum lws_callback_reasons reason,
			void *user_data, void *buf, size_t len)
{
	struct gun_context *context = (struct gun_context *)user_data;

	switch (reason) {
		// TODO
	}

	return 0;
}

static void wsi_connect_to_peer(struct lws_sorted_usec_list *sul)
{
}

static void cli_sigint_handler()
{
	running = 0;
}

int main(int argc, const char **argv)
{
	int ret = 0, n = 0;
	struct gun_context *context;
	struct lws_context_creation_info info;
	struct lws_sorted_usec_list sul;
	struct lws_context *ws_context;
	struct lws_protocols protocols[] = {
		{ "websocket", wsi_callback, 0, 0, 0, context },
		{ NULL, NULL, 0, 0, 0, NULL },
	};

	memset(&info, 0, sizeof(info));
	memset(&sul, 0, sizeof(sul));
	signal(SIGINT, cli_sigint_handler);

	lws_cmdline_option_handle_builtin(argc, argv, &info);

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;

	gun_context_new(&context);
	if ((ws_context = lws_create_context(&info)) == NULL) {
		ret = -1;
		goto out;
	}

	gun_context_add_peer(context, "ws://localhost:3033");
	gun_context_add_peer(context, "ws://localhost:3034");

	lws_sul_schedule(ws_context, 0, &sul, wsi_connect_to_peer, 1);

	while (running && n >= 0)
		n = lws_service(ws_context, 0 /*UNUSED*/);

	lws_context_destroy(ws_context);

out:
	gun_context_free(context);

	return ret;
}

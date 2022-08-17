
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <libwebsockets.h>

#include "gun.h"

static volatile int running = 1;

static void cli_sigint_handler()
{
	running = 0;
}

int main(int argc, const char **argv)
{
	int ret = 0, n = 0;
	struct gun_context *context;

	signal(SIGINT, cli_sigint_handler);

	if (gun_context_new(&context) < 0) {
		ret = -1;
		return ret;
	}

	gun_context_add_peer(context, "ws://localhost:3030");

	// lws_set_log_level(0xFFFFFFFF, NULL);

	/* main run loop */
	while (running && !context->should_abort && n >= 0)
		n = gun_com_service_request(context);

	gun_context_free(context);

	return ret;
}


#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <libwebsockets.h>

#include "log.h"

#include "gun.h"

static volatile int running = 1;

static void cli_sigint_handler()
{
	running = 0;
}

static void __gun_cli_parse_commandline(int argc, const char *argv[],
					struct gun_context *context)
{
	/*
   * TODO the getopt stuff
   *
   * usage: build/gun [-d] --peer ws://localhost:8080/gun --peer wss://something.else [-q | --quiet] [-l LOGLEVEL | --log-level LOGLEVEL]
   *
   * where
   * - d is fork into the background (run as a daemon) to be done later
   * - --peer is one or more websocket URLs of peers to connect to
   * -q --quiet: silence all logging
   * -l set log level of both libwebsockets and our own logging (see log.h for a valid list of levels)
   */
}

int main(int argc, const char *argv[])
{
	int ret = 0, n = 0;
	struct gun_context *context;

	signal(SIGINT, cli_sigint_handler);

	// TODO: getopt

	if (gun_context_new(&context) < 0) {
		ret = -1;
		return ret;
	}

	__gun_cli_parse_commandline(argc, argv, context);

	gun_context_add_peer(context, "ws://localhost:3030");

	// lws_set_log_level(0xFFFFFFFF, NULL);

	if (gun_com_start(context) < 0) {
		log_fatal("Could not connect to peers.  Do you have any?");
		goto out;
	}

	/* main run loop */
	while (running && !context->should_abort && n >= 0)
		n = gun_com_service_request(context);

out:
	gun_context_free(context);

	return ret;
}

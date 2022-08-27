#include <signal.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <libwebsockets.h>

#include "log.h"
#include "gun.h"

static volatile int running = 1;

static bool must_exit = false;

static void cli_sigint_handler()
{
	running = 0;
}

static const char *help_text =
	"Gunc  - A gun.js port to C using libwebsockets, \n"

	"ideal for running on small and embedded devices.\n\n"

	"usage:\n\n"

	"  %s --peer | -p PEER_URL [-p | --peer PEER_URL] \n"
	"       [-h | --help] [-d] [-q | --quiet] \n"
	"       [-l LOGLEVEL | --log-level LOGLEVEL]\n\n"

	"options:\n\n"
	"-h, --help                        Show this help message\n\n"
	"-p, --peer <PEER_URL>             Add a gun peer by it's url i.e. http://localhost:3030\n\n"
	"-q, --quiet                       Silence all logging\n\n"
	"-l, --log-level <LOG_LEVEL>       Optionally set the log level\n"
	"                                  Valid levels are:\n"
	"                                    - TRACE [default]\n"
	"                                    - DEBUG\n"
	"                                    - INFO\n"
	"                                    - WARN\n"
	"                                    - ERROR\n"
	"                                    - FATAl\n\n"
	"-d                                Run as daemon (currently not supported)\n\n";

static int __print_help_and_exit(char *name)
{
	size_t name_len = strlen(name);
	size_t help_len = strlen(help_text);

	char *help_str;

	size_t str_len = name_len + help_len + 1;

	if ((help_str = malloc(name_len + help_len + 1)) == NULL) {
		log_fatal(
			"Not enough memory to print help text. I'm really sorry about this :(");
		return -ENOMEM;
	}

	snprintf(help_str, str_len, help_text, name);

	printf("%s", help_str);

	free(help_str);

	must_exit = true;

	return EXIT_SUCCESS;
}

static int __gun_cli_parse_commandline(int argc, char *argv[],
				       struct gun_context *context)
{
	// If no args print help and exit
	if (argc == 1) {
		return __print_help_and_exit(argv[0]);
	}

	int c;

	while (1) {
		int option_index = 0;

		static struct option long_options[] = {
			{ "help", no_argument, 0, 'h' },
			{ "quiet", no_argument, 0, 'q' },
			{ "peer", required_argument, 0, 'p' },
			{ "log-level", required_argument, 0, 'l' },
			{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "hqp:l:d", long_options,
				&option_index);

		// break while loop once all opts parsed
		if (c == -1)
			break;

		switch (c) {
		case 'p':
			/*
			 Add a peer
			 --peer | -p
      */

			if (optarg) {
				// Could add valiation to check if a valid URI?
				gun_context_add_peer(context, optarg);
			}
			break;

		case 'q':
			/*
			 Silence all logging
			 --quiet | -q
      */

			context->opts.quiet = true;

			break;

		case 'l':
			/* 
			 Log Level
			 --log-level | -l

			 Valid levels:
       LOG_TRACE = 0, 
       LOG_DEBUG = 1, 
       LOG_INFO = 2, 
       LOG_WARN = 3, 
       LOG_ERROR = 4, 
       LOG_FATAL = 5 
     */

			if (strcmp(optarg, "TRACE") == TRACE) {
				// Trace is the default log level, so don't set it
				break;
			}

			if (strcmp(optarg, "DEBUG") == 0) {
				context->opts.log_level = DEBUG;
				break;
			}

			if (strcmp(optarg, "INFO") == 0) {
				context->opts.log_level = INFO;
				break;
			}

			if (strcmp(optarg, "WARN") == 0) {
				context->opts.log_level = WARN;
				break;
			}

			if (strcmp(optarg, "ERROR") == 0) {
				context->opts.log_level = ERROR;
				break;
			}

			if (strcmp(optarg, "FATAL") == 0) {
				context->opts.log_level = FATAL;
				break;
			}

			printf("Inavild log level '%s'", optarg);
			printf("\n\n");
			printf("Valid levels are:\n"
			       "  - TRACE [default]\n"
			       "  - DEBUG\n"
			       "  - INFO\n"
			       "  - WARN\n"
			       "  - ERROR\n"
			       "  - FATAL\n");

			return EXIT_FAILURE;

		case 'd':
			/* 
			 Run as deamon
			 -d

       To be done later
     */
			context->opts.daemon = true;

			printf("-d (Run as deamon) is currently not supported\n");
			break;

		case 'h':
			/* 
			 Help
			 --help | -h

       prints help message and exits
     */
		default:
			return __print_help_and_exit(argv[0]);
		}
	}

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	int ret = 0, n = 0;
	struct gun_context *context;

	log_info("gunc starting up");

	signal(SIGINT, cli_sigint_handler);

	if (gun_context_new(&context) < 0) {
		ret = -1;
		return ret;
	}

	__gun_cli_parse_commandline(argc, argv, context);

	if (must_exit == true) {
		goto out;
	}

	log_info("gun: our ID is %s", context->id);

	if (!context->opts.quiet) {
		lws_set_log_level(context->opts.log_level, NULL);
	}

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

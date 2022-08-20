

#include <signal.h>
#include <stdio.h>
#include <stdlib.h> /* for exit */
#include <getopt.h>
#include <string.h>
#include <libwebsockets.h>

/* 
  Not sure if this is needed here?
  
  It is in gun.h to set the bool members
*/
#include <stdbool.h>
#include "log.h"
/* 
  Not sure if this is needed here?
  
  It is in gun.h to set the bool members
*/
#include <stdbool.h>

#include "gun.h"

static volatile int running = 1;

static void cli_sigint_handler()
{
	running = 0;
}

static void __print_help_and_exit()
{
	printf("Gunc - A gun.js port to C using libwebsockets, \n"

	       "ideal for running on small and embedded devices.\n\n"

	       "usage:\n\n"

	       "  gunc --peer | -p PEER_URL [-p | --peer PEER_URL] \n"
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
	       "                                    - FATAL\n\n"

	       "-d                                Run as daemon (currently not supported)\n\n");

	/* Not sure if this is okay or if we should free stuff first?

     Looking at the docs for exit, We can add functions to atexit to
     have them called sequentially on exit?

     Keen to get your thoughts on what is best here.
  */
	exit(EXIT_SUCCESS);
}

static void __gun_cli_parse_commandline(int argc, char *argv[],
					struct gun_context *context)
{
	// If no args print help and exit
	if (argc == 1) {
		__print_help_and_exit();
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
				// log_level = 1;
				break;
			}

			if (strcmp(optarg, "INFO") == 0) {
				context->opts.log_level = INFO;
				// log_level = 2;
				break;
			}

			if (strcmp(optarg, "WARN") == 0) {
				context->opts.log_level = WARN;
				// log_level = 3;
				break;
			}

			if (strcmp(optarg, "ERROR") == 0) {
				context->opts.log_level = ERROR;
				// log_level = 4;
				break;
			}

			if (strcmp(optarg, "FATAL") == 0) {
				context->opts.log_level = FATAL;
				// log_level = 5;
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

			/* Not sure if this is okay or if we should free stuff first?

         Looking at the docs for exit, We can add functions to atexit to
         have them called sequentially on exit?

         Keen to get your thoughts on what is best here.
      */
			exit(EXIT_FAILURE);
			break;

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
			__print_help_and_exit();
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int ret = 0, n = 0;
	struct gun_context *context;

	signal(SIGINT, cli_sigint_handler);

	if (gun_context_new(&context) < 0) {
		ret = -1;
		return ret;
	}

	__gun_cli_parse_commandline(argc, argv, context);

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

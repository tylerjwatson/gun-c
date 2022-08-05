
#include <stdio.h>

#include "gun.h"

int main(int argc, char **argv)
{
	struct gun_context *context;

	gun_context_new(&context);

	gun_context_add_peer(context, "ws://localhost:3033");
	gun_context_add_peer(context, "ws://localhost:3034");

	gun_context_free(context);

	return 0;
}

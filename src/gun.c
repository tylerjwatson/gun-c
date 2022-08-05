//  SPDX-FileCopyrightText: 2022 Tyler Watson <tyler@tw.id.au>
//  SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gun.h"

int gun_context_new(struct gun_context **out_context)
{
	struct gun_context *context = malloc(sizeof(struct gun_context));

	if (context == NULL) {
		return -ENOMEM;
	}

	gun_context_init(context);

	*out_context = context;

	return 0;
}

int gun_context_init(struct gun_context *context)
{
	memset(context, 0, sizeof(*context));

	return 0;
}

int gun_context_add_peer(struct gun_context *context, const char *peer_url)
{
	struct gun_peer *peer = context->peer_list, *new_peer;

	while (peer && peer->next)
		peer = peer->next;

	if ((new_peer = malloc(sizeof(*new_peer))) == NULL) {
		return -ENOMEM;
	}

	if ((new_peer->peer = strdup(peer_url)) == NULL) {
		free(new_peer);
		return -ENOMEM;
	}

	new_peer->next = NULL;

	if (!context->peer_list) {
		context->peer_list = new_peer;
	} else {
		peer->next = new_peer;
	}

	return 0;
}

static inline void gun_context_free_peers(struct gun_context *context)
{
	struct gun_peer *n = context->peer_list, *tmp;

	while (n) {
		tmp = n;
		free(n->peer);
		n = n->next;
		free(tmp);
	}
}

void gun_context_free(struct gun_context *context)
{
	gun_context_free_peers(context);
	free(context);
}

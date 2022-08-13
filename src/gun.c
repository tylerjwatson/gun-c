//  SPDX-FileCopyrightText: 2022 Tyler Watson <tyler@tw.id.au>
//  SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gun.h"
#include "url.h"

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
	int ret = 0;

	memset(context, 0, sizeof(*context));

	if ((ret = gun_com_init(context)) < 0) {
		return ret;
	}

	return ret;
}

int gun_context_add_peer(struct gun_context *context, const char *peer_url)
{
	int ret = 0;
	struct gun_peer *peer = context->peer_list, *new_peer;
	struct yuarel *url;

	while (peer && peer->next)
		peer = peer->next;

	if ((new_peer = malloc(sizeof(*new_peer))) == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	if ((new_peer->peer_data = strdup(peer_url)) == NULL) {
		ret = -ENOMEM;
		goto peer_out;
	}

	if ((new_peer->url = malloc(sizeof(struct yuarel))) == NULL) {
		ret = -ENOMEM;
		goto peer_data_out;
	}

	/* According to yuarel documentation the parse function
   * modifies the string passed into it, so use a copy
   * here.
   */
	if (yuarel_parse(new_peer->url, new_peer->peer_data) == -1) {
		ret = -100; // TODO: error enums and handling
		goto url_out;
	}

	new_peer->next = NULL;

	if (!context->peer_list) {
		context->peer_list = new_peer;
	} else {
		peer->next = new_peer;
	}

	ret = 0;
	goto out;

url_out:
	free(new_peer->url);

peer_data_out:
	free(new_peer->peer_data);

peer_out:
	free(new_peer);

out:
	return ret;
}

static inline void gun_context_free_peers(struct gun_context *context)
{
	struct gun_peer *n = context->peer_list, *tmp;

	while (n) {
		tmp = n;
		free(n->url);
		free(n->peer_data);
		n = n->next;
		free(tmp);
	}
}

void gun_context_free(struct gun_context *context)
{
	if (context->ws_context)
		lws_context_destroy(context->ws_context);
	gun_context_free_peers(context);
	free(context);
}

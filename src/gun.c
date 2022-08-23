//  SPDX-FileCopyrightText: 2022 Tyler Watson <tyler@tw.id.au>
//  SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gun.h"
#include "url.h"
#include "log.h"
#include "dup.h"
#include "mjson.h"

#define MAX_ID_LENGTH 256

static void __gun_on_message(struct gun_peer *peer, size_t len, const char *msg)
{
	char id[32];
	size_t id_len;

	if (len <= 0 || len > GUN_MAX_MSG_LENGTH) {
		return;
	}

	if (mjson_find(msg, len, "$", NULL, NULL) == MJSON_TOK_ARRAY) {
		int koff, klen, voff, vlen, vtype, off;

		for (off = 0; (off = mjson_next(msg, len, off, &koff, &klen,
						&voff, &vlen, &vtype)) != 0;) {
			__gun_on_message(peer, vlen, msg + voff);
			// printf("blank key: %.*s, value: %.*s type %d\n", klen,
			//        json + koff, vlen, json + voff, vtype);
		}

		return;
	}

	if ((id_len = mjson_get_string(msg, len, "$.#", id, sizeof(id)) ==
		      -1)) {
		log_error("gun: item does not appear to be a gun message.");
		return;
	}

	if (gun_dup_check(&peer->context->dup, id)) {
		log_debug("dup: seen this id before, discarding.");
		return;
	}

	gun_dup_track(&peer->context->dup, id);

	log_trace("gun: message peer=%s:%d id=%s contents=%s", peer->url->host,
		  peer->url->port, id, msg);

	// TODO: forward to our peers
}

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

	context->on_message = __gun_on_message;

	context->opts.log_level = TRACE;

	if ((ret = gun_com_init(context)) < 0) {
		return ret;
	}

	if (gun_dup_init(context, &context->dup, 900) < 0) {
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

	memset(new_peer, 0, sizeof(*new_peer));

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

	new_peer->context = context;
	new_peer->next = NULL;

	if (!context->peer_list)
		context->peer_list = new_peer;
	else
		peer->next = new_peer;

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
	gun_dup_context_free(&context->dup);
	if (context->ws_context)
		lws_context_destroy(context->ws_context);
	gun_context_free_peers(context);
	free(context);
}

//  SPDX-FileCopyrightText: 2022 Tyler Watson <tyler@tw.id.au>
//  SPDX-License-Identifier: MIT

#include "gun.h"
#include "url.h"
#include <stdio.h>

static int wsi_callback(struct lws *wsi, enum lws_callback_reasons reason,
			void *user_data, void *buf, size_t len)
{
	struct gun_context *context = (struct gun_context *)user_data;

	switch (reason) {
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		fprintf(stderr, "connection error: %s\n", (char *)buf);
		context->should_abort = 1;
		break;
		// TODO
	default:
		break;
	}

	return 0;
}

static struct lws_protocols protocols[] = {
	{ "gun", wsi_callback, 0, 0 },
	{ NULL, NULL, 0, 0, 0, NULL },
};

static void wsi_connect_to_peer(struct lws_sorted_usec_list *sul)
{
	struct gun_context *context =
		lws_container_of(sul, struct gun_context, sul);
	struct lws_client_connect_info info = { 0 };
	const struct gun_peer *peer = context->peer_list;

	info.context = context->ws_context;
	info.port = peer->url->port;
	info.address = peer->url->host;
	info.path = info.origin = peer->url->path;
	info.ssl_connection = 0;
	info.protocol = "gun";
	info.local_protocol_name = "gun";
	info.pwsi = &context->lws;
	info.userdata = context;

	if (lws_client_connect_via_info(&info) == NULL) {
		lws_retry_sul_schedule(context->ws_context, 0, sul, NULL,
				       wsi_connect_to_peer, NULL);
	}
}

int gun_com_init(struct gun_context *context)
{
	int ret = 0;
	struct lws_context_creation_info info = { 0 };

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;

	if ((context->ws_context = lws_create_context(&info)) == NULL) {
		ret = -1;
		goto out;
	}

out:
	return ret;
}

int gun_com_service_request(struct gun_context *context)
{
	lws_sul_schedule(context->ws_context, 0, &context->sul,
			 wsi_connect_to_peer, 1);

	return lws_service(context->ws_context, 0 /*UNUSED*/);
}

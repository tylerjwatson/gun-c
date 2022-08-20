//  SPDX-FileCopyrightText: 2022 Tyler Watson <tyler@tw.id.au>
//  SPDX-License-Identifier: MIT

#include <stdio.h>
#include <errno.h>

#include "gun.h"
#include "url.h"
#include "log.h"

static int wsi_callback(struct lws *wsi, enum lws_callback_reasons reason,
			void *user_data, void *buf, size_t len);

static struct lws_protocols protocols[] = {
	{ "gun", wsi_callback, 0, 0 },
	{ NULL, NULL, 0, 0 },
};

static void wsi_connect_to_peer(struct lws_sorted_usec_list *sul)
{
	struct gun_peer *peer = lws_container_of(sul, struct gun_peer, sul);
	struct gun_context *context = peer->context;
	struct lws_client_connect_info info = { 0 };

	info.context = context->ws_context;
	info.port = peer->url->port;
	info.address = peer->url->host;
	info.host = lws_canonical_hostname(context->ws_context);
	info.path = info.origin = "/gun";
	info.ssl_connection = 0;
	info.protocol = "gun";
	info.local_protocol_name = "gun";
	info.pwsi = &context->lws;
	info.userdata = peer;

	if (lws_client_connect_via_info(&info) == NULL) {
		lws_retry_sul_schedule(context->ws_context, 0, sul, NULL,
				       wsi_connect_to_peer, NULL);
	}
}

static int wsi_callback(struct lws *wsi, enum lws_callback_reasons reason,
			void *user_data, void *buf, size_t len)
{
	struct gun_peer *peer = (struct gun_peer *)user_data;
	struct gun_context *context;
	
	if (peer)
		context = peer->context;

	switch (reason) {
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		log_info("connection established with peer %s:%d",
			 peer->url->host, peer->url->port);
		break;
	case LWS_CALLBACK_CLIENT_RECEIVE:
		log_trace("rx msg=%s len=%d", (const char *)buf, len);

		if (context->on_message != NULL) {
			context->on_message(context, len, (const char *)buf);
		}
		break;
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		log_error("com: peer %s:%d connection error: %s",
			  peer->url->host, peer->url->port, (char *)buf);
		/* fall through */
	case LWS_CALLBACK_CLIENT_CLOSED:
		lws_retry_sul_schedule_retry_wsi(
			wsi, &peer->sul, wsi_connect_to_peer, &peer->retries);
		break;
	default:
		break;
	}

	return 0;
}

int gun_com_init(struct gun_context *context)
{
	int ret = 0;
	struct lws_context_creation_info info = { 0 };
	struct gun_peer *peer = context->peer_list;

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;

	if ((context->ws_context = lws_create_context(&info)) == NULL) {
		ret = -1;
		goto out;
	}

out:
	return ret;
}

int gun_com_start(struct gun_context *context)
{
	struct gun_peer *peer = context->peer_list;
	int i = 0;

	while (peer) {
		if (i > 100) {
			log_fatal(
				"com: connecting to over 100 peers, something's probably wrong.");
			context->should_abort = 1;
			return -1;
		}

		log_info("com: connecting to peer %s:%d", peer->url->host,
			 peer->url->port);
		lws_sul_schedule(context->ws_context, 0, &peer->sul,
				 wsi_connect_to_peer, ++i * 1000);

		peer = peer->next;
	}

	return 0;
}

int gun_com_service_request(struct gun_context *context)
{
	return lws_service(context->ws_context, 0);
}

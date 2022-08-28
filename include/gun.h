//  SPDX-FileCopyrightText: 2022 Tyler Watson <tyler@tw.id.au>
//  SPDX-License-Identifier: MIT

#pragma once

#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <libwebsockets.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GUN_MAX_MSG_LENGTH (256 * 1024 * 1024)

// forward declarations
struct yuarel;
struct gun_peer;
struct ht;

/**
 * Callback that happens when a message has been received from the
 * com module.
 */
typedef void (*gun_msg_cb_t)(struct gun_peer *peer, size_t msg_len,
			     const char *msg);

struct gun_dup_context {
	unsigned ttl;
	struct lws_context *ws_context;
	struct lws_sorted_usec_list sul;
	struct ht *id_table;
};

struct gun_peer {
  char id[16];
	void *peer_data;
	struct gun_context *context;
	struct yuarel *url;
	struct lws_sorted_usec_list sul;
	struct lws *ws_handle;
	struct gun_peer *next;
	uint16_t retries;
};

enum LOG_LEVEL { TRACE, DEBUG, INFO, WARN, ERROR, FATAL };

struct gun_context_options {
	int log_level;
	bool quiet;
	bool daemon;
};

struct gun_context {
  char id[16];
	struct gun_peer *peer_list;
	struct lws_context *ws_context;
	struct lws_sorted_usec_list sul;
	struct gun_dup_context dup;
	gun_msg_cb_t on_message;

	struct gun_context_options opts;

	volatile int should_abort;
};

struct gun_node {};

/**
  * Generates a new ID of the specified length into the pointer
  * pointed to by id.
  *
  * The pointer pointed to by id must be allocated and
  * have enough space for the characters plus the null byte.
  */
void gun_generate_random_string(size_t len, char *id);

int gun_context_new(struct gun_context **out_context);

/**
  * Initializes a gun context.
  */
int gun_context_init(struct gun_context *context);

/**
  * Adds a new peer to the provided gun context.  The string pointed to by
  * peer will be copied into the peer list and retained until gun_context_free
  * has been called.
  */
int gun_context_add_peer(struct gun_context *context, const char *peer);

/**
  * Frees a gun context and all associated memoey.
  */
void gun_context_free(struct gun_context *context);

/*********************** COM **************************/

/**
  * Initialize COM - the gun communication layer.
  * Requries a pointer to a fully-initialized gun
  * context.
  */
int gun_com_init(struct gun_context *context);

/**
  * Start connecting to peers in the peer list.
  *
  * Requries a fully initialized COM interface, call gun_com_init
  * first.
  */
int gun_com_start(struct gun_context *context);

int gun_com_service_request(struct gun_context *context);

/*********************** DUP **************************/

#ifdef __cplusplus
}
#endif

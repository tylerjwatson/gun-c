//  SPDX-FileCopyrightText: 2022 Tyler Watson <tyler@tw.id.au>
//  SPDX-License-Identifier: MIT

#pragma once

#include <errno.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct gun_peer {
	char *peer;
	struct gun_peer *next;
};

struct gun_context {
	struct gun_peer *peer_list;
};

struct gun_node {};

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

#ifdef __cplusplus
}
#endif

#pragma once

#include <time.h>
#include <stdbool.h>

#include "gun.h"

struct gun_dup_entry {
	time_t expiry;
};

int gun_dup_init(const struct gun_context *gun_context,
		 struct gun_dup_context *context, unsigned ttl);

/**
  * Tracks whether a message ID has already been seen.
  *
  * Returns 1 if the id has been seen before and not expired, 0
  * if it has not been seen yet and has been tracked, and
  * < 0 if there was an allocation error.
  *
  * This function allocates memory.
  */
int gun_dup_track(struct gun_dup_context *context, const char *id);

/**
  * Checks if a request ID is tracked by DUP, and if it has,
  * update its time to live via gun_dup_track;
  */
bool gun_dup_check(struct gun_dup_context *context, const char *id);

void gun_dup_context_free(struct gun_dup_context *context);

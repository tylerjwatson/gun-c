
//  SPDX-FileCopyrightText: 2022 Tyler Watson <tyler@tw.id.au>
//  SPDX-License-Identifier: MIT

#include "gun.h"
#include "dup.h"
#include "ht.h"
#include "log.h"
#include <time.h>

int gun_dup_init(struct gun_dup_context *context, unsigned ttl)
{
	if ((context->id_table = ht_create()) == NULL) {
		log_fatal("dup: cannot allocate hashtable; out of memory.\n");
		return -ENOMEM;
	}

	log_debug("dup: init hashtable with %d entries and %d ttl\n",
		  context->id_table->capacity, ttl);

	context->ttl = ttl;

	return 0;
}

int gun_dup_track(struct gun_dup_context *context, const char *id)
{
	int ret = 0;
	struct gun_dup_entry *entry =
		(struct gun_dup_entry *)ht_get(context->id_table, id);
	time_t now = time(NULL);
	double age;

	if (entry) {
		time(&now);
		if ((age = difftime(now, entry->expiry)) < 0) {
			log_debug(
				"dup: %s: seen it before but it expired %d secs ago,"
				"entry is trash and we will clean it up and act like "
				"we've never seen it.\n",
				id, age * -1);
			entry = (struct gun_dup_entry *)ht_remove(
				context->id_table, id);
			free(entry);
			entry = NULL;
		} else {
			ret = 1; // seen it
		}
	}

	if (!entry) {
		log_debug("dup: %s: not seen this ID before", id);
		if ((entry = malloc(sizeof(struct gun_dup_entry))) == NULL) {
			log_fatal("dup: out of memory allocating dup entry\n");
			return -ENOMEM;
		}
		ht_set(context->id_table, id, entry);
	}

	entry->expiry = now + context->ttl;

	return ret;
}

bool gun_dup_check(struct gun_dup_context *context, const char *id)
{
	struct gun_dup_entry *entry =
		(struct gun_dup_entry *)ht_get(context->id_table, id);

	if (entry == NULL) {
		return NULL;
	}

	return gun_dup_track(context, id);
}

static inline void __gun_dup_free_entry(void *entry)
{
	free(entry);
}

void gun_dup_context_free(struct gun_dup_context *context)
{
	struct ht_iterator iter = ht_iterator(context->id_table);
	struct gun_dup_entry entry;

	while (ht_next(&iter))
		__gun_dup_free_entry(iter.value);

	ht_destroy(context->id_table);
}

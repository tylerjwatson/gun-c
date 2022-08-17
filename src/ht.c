// Simple hash table implemented in C.

#include "ht.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct ht *ht_create(void)
{
	// Allocate space for hash table struct.
	struct ht *table = malloc(sizeof(struct ht));
	if (table == NULL) {
		return NULL;
	}
	table->length = 0;
	table->capacity = INITIAL_CAPACITY;

	// Allocate (zero'd) space for entry buckets.
	table->entries = calloc(table->capacity, sizeof(struct ht_entry));
	if (table->entries == NULL) {
		free(table); // error, free table before we return!
		return NULL;
	}
	return table;
}

void ht_destroy(struct ht *table)
{
	// First free allocated keys.
	for (size_t i = 0; i < table->capacity; i++) {
		free((void *)table->entries[i].key);
	}

	// Then free entries array and table itself.
	free(table->entries);
	free(table);
}

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

// Return 64-bit FNV-1a hash for key (NUL-terminated). See description:
// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
static uint64_t hash_key(const char *key)
{
	uint64_t hash = FNV_OFFSET;
	for (const char *p = key; *p; p++) {
		hash ^= (uint64_t)(unsigned char)(*p);
		hash *= FNV_PRIME;
	}
	return hash;
}

void *ht_get(struct ht *table, const char *key)
{
	// AND hash with capacity-1 to ensure it's within entries array.
	uint64_t hash = hash_key(key);
	size_t index = (size_t)(hash & (uint64_t)(table->capacity - 1));

	// Loop till we find an empty entry.
	while (table->entries[index].key != NULL) {
		if (strcmp(key, table->entries[index].key) == 0) {
			// Found key, return value.
			return table->entries[index].value;
		}
		// Key wasn't in this slot, move to next (linear probing).
		index++;
		if (index >= table->capacity) {
			// At end of entries array, wrap around.
			index = 0;
		}
	}
	return NULL;
}

static void *ht_remove_entry(struct ht_entry *entries, size_t capacity,
			     const char *key)
{
	// AND hash with capacity-1 to ensure it's within entries array.
	uint64_t hash = hash_key(key);
	size_t index = (size_t)(hash & (uint64_t)(capacity - 1));
	void *removed_value = NULL;

	while (entries[index].key != NULL) {
		if (strcmp(key, entries[index].key) == 0) {
			// Found key (it already exists), free entry
			free((void *)entries[index].key);
			removed_value = entries[index].value;

			entries[index].key = NULL;
			entries[index].value = NULL;

			goto out;
		}
		// Key wasn't in this slot, move to next (linear probing).
		index++;
		if (index >= capacity) {
			// At end of entries array, wrap around.
			index = 0;
		}
	}

out:
	return removed_value;
}

// Internal function to set an entry (without expanding table).
static const char *ht_set_entry(struct ht_entry *entries, size_t capacity,
				const char *key, void *value, size_t *plength)
{
	// AND hash with capacity-1 to ensure it's within entries array.
	uint64_t hash = hash_key(key);
	size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

	// Loop till we find an empty entry.
	while (entries[index].key != NULL) {
		if (strcmp(key, entries[index].key) == 0) {
			// Found key (it already exists), update value.
			entries[index].value = value;
			return entries[index].key;
		}
		// Key wasn't in this slot, move to next (linear probing).
		index++;
		if (index >= capacity) {
			// At end of entries array, wrap around.
			index = 0;
		}
	}

	// Didn't find key, allocate+copy if needed, then insert it.
	if (plength != NULL) {
		key = strdup(key);
		if (key == NULL) {
			return NULL;
		}
		(*plength)++;
	}
	entries[index].key = (char *)key;
	entries[index].value = value;
	return key;
}

// Expand hash table to twice its current size. Return true on success,
// false if out of memory.
static bool ht_expand(struct ht *table)
{
	// Allocate new entries array.
	size_t new_capacity = table->capacity * 2;
	if (new_capacity < table->capacity) {
		return false; // overflow (capacity would be too big)
	}
	struct ht_entry *new_entries =
		calloc(new_capacity, sizeof(struct ht_entry));
	if (new_entries == NULL) {
		return false;
	}

	// Iterate entries, move all non-empty ones to new table's entries.
	for (size_t i = 0; i < table->capacity; i++) {
		struct ht_entry entry = table->entries[i];
		if (entry.key != NULL) {
			ht_set_entry(new_entries, new_capacity, entry.key,
				     entry.value, NULL);
		}
	}

	// Free old entries array and update this table's details.
	free(table->entries);
	table->entries = new_entries;
	table->capacity = new_capacity;
	return true;
}

void *ht_remove(struct ht *table, const char *key)
{
	assert(key != NULL);
	return ht_remove_entry(table->entries, table->capacity, key);
}

const char *ht_set(struct ht *table, const char *key, void *value)
{
	assert(value != NULL);
	if (value == NULL) {
		return NULL;
	}

	// If length will exceed half of current capacity, expand it.
	if (table->length >= table->capacity / 2) {
		if (!ht_expand(table)) {
			return NULL;
		}
	}

	// Set entry and update length.
	return ht_set_entry(table->entries, table->capacity, key, value,
			    &table->length);
}

size_t ht_length(struct ht *table)
{
	return table->length;
}

struct ht_iterator ht_iterator(struct ht *table)
{
	struct ht_iterator it;
	it._table = table;
	it._index = 0;
	return it;
}

bool ht_next(struct ht_iterator *it)
{
	// Loop till we've hit end of entries array.
	struct ht *table = it->_table;
	while (it->_index < table->capacity) {
		size_t i = it->_index;
		it->_index++;
		if (table->entries[i].key != NULL) {
			// Found next non-empty item, update iterator key and value.
			struct ht_entry entry = table->entries[i];
			it->key = entry.key;
			it->value = entry.value;
			return true;
		}
	}
	return false;
}

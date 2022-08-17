// Simple hash table implemented in C.

#ifndef _HT_H
#define _HT_H

#include <stdbool.h>
#include <stddef.h>

// Hash table entry (slot may be filled or empty).
struct ht_entry {
	const char *key; // key is NULL if this slot is empty
	void *value;
};

// Hash table structure: create with ht_create, free with ht_destroy.
struct ht {
	struct ht_entry *entries; // hash slots
	size_t capacity; // size of _entries array
	size_t length; // number of items in hash table
};

#define INITIAL_CAPACITY 16 // must not be zero

// Create hash table and return pointer to it, or NULL if out of memory.
struct ht *ht_create(void);

// Free memory allocated for hash table, including allocated keys.
void ht_destroy(struct ht *table);

// Get item with given key (NUL-terminated) from hash table. Return
// value (which was set with ht_set), or NULL if key not found.
void *ht_get(struct ht *table, const char *key);

// Set item with given key (NUL-terminated) to value (which must not
// be NULL). If not already present in table, key is copied to newly
// allocated memory (keys are freed automatically when ht_destroy is
// called). Return address of copied key, or NULL if out of memory.
const char *ht_set(struct ht *table, const char *key, void *value);

// Removes a key from the hashtable and returns a pointer to the
// value that was removed if any exists, and NULL if an entry
// matching `key` was not found or the entry's value also pointed
// to NULL.
void *ht_remove(struct ht *table, const char *key);

// Return number of items in hash table.
size_t ht_length(struct ht *table);

// Hash table iterator: create with ht_iterator, iterate with ht_next.
struct ht_iterator {
	const char *key; // current key
	void *value; // current value

	// Don't use these fields directly.
	struct ht *_table; // reference to hash table being iterated
	size_t _index; // current index into ht._entries
};

// Return new hash table iterator (for use with ht_next).
struct ht_iterator ht_iterator(struct ht *table);

// Move iterator to next item in hash table, update iterator's key
// and value to current item, and return true. If there are no more
// items, return false. Don't call ht_set during iteration.
bool ht_next(struct ht_iterator *it);

#endif // _HT_H

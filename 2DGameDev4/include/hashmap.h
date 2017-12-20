#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include "gf2d_vector.h"
#include <SDL.h>

typedef struct KeyValue_s
{
	char *key;
	void *value;
} KeyValue;

typedef struct HashMap_s
{
	KeyValue *entries;
	int maxentries;

	int chunksize;
	int numchunks;

	int numentries;
} HashMap;

int hash(char *key);

HashMap *hashmap_new(Uint32 numchunks, Uint32 chunksize);

HashMap *hashmap_insert(char *key, void *data, HashMap *hashmap, int(*hash)());

void *hashmap_get_value(char *key, HashMap *hashmap);

void hashmap_print_keys(HashMap *hashmap);

void hashmap_delete(HashMap *hashmap, void(*free_value)(void *value));

#endif
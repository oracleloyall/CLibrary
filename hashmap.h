/*
 *Data :2017 2 28
 */
#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#define MAP_MISSING -3  /* No such element */
#define MAP_FULL -2 	/* Hashmap is full */
#define MAP_OMEM -1 	/* Out of Memory */
#define MAP_OK 0 	/* OK */


typedef void *any_t;


typedef int (*PFany)(any_t, any_t);

typedef any_t map_t;

/*
 * Return an empty hashmap. Returns NULL if empty.
*/
extern map_t hashmap_new();

extern int hashmap_iterate(map_t in, PFany f, any_t item);

/*
 * Add an element to the hashmap.
 */
extern int hashmap_put(map_t in, char* key, any_t value);

/*
 * Get an element from the hashmap
 */
extern int hashmap_get(map_t in, char* key, any_t *arg);

/*
 * Remove an element from the hashmap.
 */
extern int hashmap_remove(map_t in, char* key);

/*
 * Get any element.
 *
 */
extern int hashmap_get_one(map_t in, any_t *arg, int remove);

/*
 * Free the hashmap
 */
extern void hashmap_free(map_t in);

/*
 * Get the current size of a hashmap
 */
extern int hashmap_length(map_t in);

#endif __HASHMAP_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HASHMAP_SIZE 256

typedef struct node_s node_t;

typedef struct node_s {
  char *key;
  node_t *next;
} node_t;

typedef struct {
  node_t **nodes;
  unsigned int count;
  unsigned int capacity;
  unsigned long long collisions;
} hashmap_t;

unsigned int hash(char *key, size_t capacity) {
  unsigned int hash = 5381;
  for (int i = 0; key[i] != '\0'; i++) {
    hash = hash * 31 + key[i];
  }
  return hash % capacity;
}

hashmap_t *hashmap_init(size_t capacity) {
  if (capacity == 0)
    return NULL;

  hashmap_t *map = (hashmap_t *)malloc(sizeof(hashmap_t));
  map->nodes = (node_t **)calloc(capacity, sizeof(node_t *));
  map->capacity = capacity;
  return map;
}

void hashmap_insert(hashmap_t *map, char *key) {
  unsigned int index = hash(key, map->capacity);
  node_t *node = map->nodes[index];
  if (node == NULL) {
    node = (node_t *)malloc(sizeof(node_t));
    node->key = strdup(key);
    node->next = NULL;
    map->nodes[index] = node;
    map->count++;
    return;
  }

  map->collisions++;
  node_t *prev = NULL;
  while (node != NULL) {
    if (strcmp(node->key, key) == 0)
      return;

    prev = node;
    node = node->next;
  }

  node = (node_t *)malloc(sizeof(node_t));
  node->key = strdup(key);
  node->next = NULL;
  prev->next = node;
  map->count++;
}

char *hashmap_search(hashmap_t *map, char *key) {
  unsigned int index = hash(key, map->capacity);
  node_t *node = map->nodes[index];
  if (node == NULL)
    return NULL;

  while (node != NULL) {
    if (strcmp(node->key, key) == 0)
      return node->key;

    node = node->next;
  }

  return NULL;
}

void hashmap_dump(hashmap_t *map, int collisions) {
  for (size_t i = 0; i < map->capacity; i++) {
    node_t *node = map->nodes[i];
    if (node == NULL)
      continue;

    if (collisions && node->next == NULL)
      continue;

    printf("[%zu]: ", i);
    while (node != NULL) {
      printf("(%s) -> ", node->key);
      node = node->next;
    }

    printf("NULL\n");
  }
}

int main(int argc, char *argv[]) {
  FILE *file = fopen("navn.txt", "r");
  if (!file)
    return -1;

  char line[256];
  char *names[256];
  int count = 0;

  while (fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = 0;
    names[count] = strdup(line);
    count++;
  }

  fclose(file);
  hashmap_t *map = hashmap_init(HASHMAP_SIZE);
  if (!map) {
    fprintf(stderr, "Failed to initialize hashmap\n");
    return -1;
  }

  for (int i = 0; i < count; i++) {
    hashmap_insert(map, names[i]);
  }

  printf("\nHashmap contents:\n");
  hashmap_dump(map, 0);
  printf("\n");

  printf("\nCollisions only:\n");
  hashmap_dump(map, 1);
  printf("\n");

  char *search_key = "Gustav Haverstad,Skyberg";
  char *result = hashmap_search(map, search_key);
  if (result) {
    printf("Found: %s\n", result);
  } else {
    printf("Not found: %s\n", search_key);
  }

  unsigned long long load_factor = (map->count * 100) / (unsigned long long)map->capacity;
  unsigned long long collision_rate = (map->collisions * 100) / (unsigned long long)map->count;
  printf("Load factor: %llu%%\n", load_factor);
  printf("Collisions: %llu\n", map->collisions);
  printf("Collision rate: %llu%%\n", collision_rate);

  return 0;
}

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct node_s node_t;

typedef struct node_s {
  char *key;
  char *value;
  node_t *next;
} node_t;

typedef struct {
  node_t **nodes;
  size_t capacity;
} hashmap_t;

hashmap_t *hashmap_init(size_t capacity) {
  hashmap_t *map = (hashmap_t *)malloc(sizeof(hashmap_t));
  if (!map)
    return NULL;

  map->nodes = (node_t **)malloc(sizeof(node_t *) * capacity);
  if (!map->nodes)
    return NULL;

  map->capacity = capacity;
  return map;
}

unsigned int hash(char *key, size_t capacity) {
  unsigned int sum = 0;
  for (int i = 0; key[i] != '\0'; i++) {
    sum += sum * 37 + key[i];
  }

  return sum % capacity;
}

node_t *node_init(char *key, char *value, unsigned int *index) {
  node_t *node = (node_t *)malloc(sizeof(node_t));
  if (!node)
    return NULL;

  node->key = key;
  node->value = value;
  node->next = NULL;
  return node;
}

void hashmap_insert(hashmap_t *map, char *key, char *value) {
  unsigned int index = hash(key, map->capacity);
  node_t *node = map->nodes[index];
  if (node == NULL) {
    map->nodes[index] = node_init(key, value, &index);
    return;
  }

  node_t *prev = NULL;
  while (node != NULL) {
    if (strcmp(node->key, key) == 0) {
      node->value = value;
      return;
    }

    prev = node;
    node = node->next;
  }

  prev->next = node_init(key, value, &index);
}

char *hashmap_get(hashmap_t *map, char *key) {
  unsigned int index = hash(key, map->capacity);
  node_t *node = map->nodes[index];
  if (node == NULL)
    return NULL;

  while (node != NULL) {
    if (strcmp(node->key, key) == 0)
      return node->value;

    node = node->next;
  }

  return NULL;
}

void hashmap_dump(hashmap_t *map) {
  for (size_t i = 0; i < map->capacity; i++) {
    node_t *node = map->nodes[i];
    if (node == NULL)
      continue;

    printf("[%zu]: ", i);
    while (node != NULL) {
      printf("(%s: %s) -> ", node->key, node->value);
      node = node->next;
    }

    printf("NULL\n");
  }
}

int main(int argc, char *argv[]) {
  hashmap_t *map = hashmap_init(3);
  hashmap_insert(map, "temp1", "Alice");
  hashmap_insert(map, "temp2", "Bob");
  hashmap_insert(map, "temp3", "Charlie");
  hashmap_insert(map, "temp4", "David");
  hashmap_insert(map, "temp5", "Eve");
  hashmap_dump(map);
  
  char *value = hashmap_get(map, "temp4");
  printf("Value for 'temp4': %s\n", value ? value : "Not found");

  return 0;
}

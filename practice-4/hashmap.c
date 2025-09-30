#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
  char *key;
  char *value;
  struct node_t *next;
} node_t;

void node_init(node_t *node, char *key, void *value) {
  node->key = key;
  node->value = value;
  node->next = NULL;
}

typedef struct {
  node_t **nodes;
  size_t size;
  size_t capacity;
} hashmap_t;

int hashmap_init(hashmap_t *map, size_t capacity) {
  map->nodes = (node_t **)malloc(sizeof(node_t *) * capacity);
  if (!map->nodes)
    return -1;

  map->size = 0;
  map->capacity = capacity;
  return 0;
}

int hash(char *key, size_t capacity) {
  int sum = 0, factor = 0;
  for (int i = 0; key[i] != '\0'; i++) {
    sum = (sum % capacity + ((int)key[i] * factor) % capacity) % capacity;
    factor = ((factor % __INT32_MAX__) * 31 % __INT32_MAX__) % __INT32_MAX__;
  }

  return sum;
}

int hashmap_put(hashmap_t *map, char *key, char *value) {
  int index = hash(key, map->capacity);
  node_t *new_node = (node_t *)malloc(sizeof(node_t));
  if (!new_node)
    return -1;

  node_init(new_node, key, value);
  if (map->nodes[index] == NULL) {
    map->nodes[index] = new_node;
    return 0;
  }

  new_node->next = map->nodes[index];
  map->nodes[index] = new_node;
  return 0;
}



int main(int argc, char *argv[]) {

    return 0;
}

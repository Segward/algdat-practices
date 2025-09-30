#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct node node_t;

typedef struct node{
  char *key;
  char *value;
  node_t *next;
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

int hashmap_insert(hashmap_t *map, char *key, char *value) {
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

node_t *hashmap_get(hashmap_t *map, char *key) {
  int index = hash(key, map->capacity);
  node_t *node = map->nodes[index];

  node_t *head = NULL, *tail = NULL;
  while (node != NULL) {
    if (strcmp(node->key, key) == 0) {
      node_t *copy = (node_t *)malloc(sizeof(node_t));
      if (!copy)
        return NULL;

      node_init(copy, node->key, node->value);
      if (head == NULL) {
        head = copy;
        tail = copy;
      } else {
        tail->next = copy;
        tail = copy;
      }
    }

    node = node->next;
  }

  return head;
}

int main(int argc, char *argv[]) {
  hashmap_t *map = (hashmap_t *)malloc(sizeof(hashmap_t));
  if (hashmap_init(map, 1000) != 0) {
    fprintf(stderr, "Failed to initialize hashmap\n");
    return -1;
  }

  hashmap_insert(map, "test", "Alice");
  hashmap_insert(map, "test2", "Bob");
  hashmap_insert(map, "test3", "Charlie");
  hashmap_insert(map, "test2", "David");

  node_t *test2 = hashmap_get(map, "test2");
  while (test2 != NULL) {
    printf("Key: %s, Value: %s\n", test2->key, test2->value);
    test2 = test2->next;
  }

  return 0;
}

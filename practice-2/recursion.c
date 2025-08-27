#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

typedef struct {
  int value;
  int key;
  node_t *next;
} node_t;

typedef struct {
  node_t *head;
  size_t size;
} hashmap_t;

int hashmap_create(hashmap_t *hm, size_t size) {
  if (size == 0 || hm == NULL)
    return -1;

  hm->head = (node_t *)malloc(sizeof(node_t) * size);
  if (hm->head == NULL)
    return -1;

  hm->size = size;
  memset(hm->head, 0, sizeof(node_t) * size);
  return 0;
}

int hash(int key, size_t size) {
  return abs(key) % size;
}

int hashmap_insert(hashmap_t *hm, int key, int value) {
  if (hm == NULL || hm->head == NULL)
    return -1;

  int index = hash(key, hm->size);
  node_t *new_node = (node_t *)malloc(sizeof(node_t));
  if (new_node == NULL)
    return -1;

  new_node->key = key;
  new_node->value = value;
  new_node->next = hm->head[index];
  hm->head[index] = new_node;
  return 0;
}

int hashmap_get(hashmap_t *hm, int key, int *value) {
  if (hm == NULL || hm->head == NULL || value == NULL)
    return -1;
  
  int index = hash(key, hm->size);
  node_t *node = hm->head[index];
  while (node != NULL) {
    if (node->key == key) {
      *value = node->value;
      return 0;
    }
  }

  return -1;
}

int multiply_foo(int a, int b) {
  if (a == 1)
    return b;

  return b + multiply_foo(a - 1, b);
}

#define IS_EVEN(n) ((n) % 2 == 0)

int multiply_bar(int a, int b) {
  if (b == 1)
    return a;
  
  if (IS_EVEN(b))
    return multiply_bar(a/2, b + b);

  return a + multiply_bar((a - 1)/2, b + b);
}

int main(const int argc, const char *argv[]) {

    return 0;
}

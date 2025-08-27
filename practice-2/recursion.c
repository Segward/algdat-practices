#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#define LOG_RES(T) \
  printf("CPU clock resolution: %lis %li ns\n", (T).res.tv_sec, (T).res.tv_nsec)

#define LOG_TIME(T) \
  printf("Elapsed time: %d s %li ns\n", (T).sec, (T).nsec)

typedef struct _node {
    int key;
    int value;
    struct _node* next;
} node_t;

typedef struct {
  node_t **head;
  size_t size;
} hashmap_t;

int hashmap_create(hashmap_t *hm, size_t size) {
  if (size == 0 || hm == NULL)
    return -1;

  hm->head = (node_t **)malloc(sizeof(node_t*) * size);
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

double multiply_foo(int a, double b) {
  if (a == 1)
    return b;

  return b + multiply_foo(a - 1, b);
}

#define IS_EVEN(n) ((n) % 2 == 0)

double multiply_bar(int a, double b) {
  if (a == 1)
    return b;
  
  if (IS_EVEN(a))
    return multiply_bar(a/2, b + b);

  return b + multiply_bar((a - 1)/2, b + b);
}

typedef struct {
  struct timespec res;
  struct timespec start;
  struct timespec end;
  int sec;
  long nsec;
} timer_t;

int time_func(int a, double b, double (*func)(int, double), timer_t *t) {
  if (t == NULL || func == NULL)
    return -1;

  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t->start);
  double result = func(a, b);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t->end);
  t->sec = t->end.tv_sec - t->start.tv_sec;
  t->nsec = t->end.tv_nsec - t->start.tv_nsec;
 
  if (t->nsec < 0) {
    t->sec -= 1;
    t->nsec += 1000000000;
  }

  printf("Result: %f\n", result);
  LOG_TIME(*t);
  return 0; 
}

int time_funcs(int a, double b, double (**funcs)(int, double), size_t size) {
  if (funcs == NULL || size == 0)
    return -1;

  timer_t timer = {0};
  for (size_t i = 0; i < size; i++) {
    time_func(a, b, funcs[i], &timer);
  }

  return 0;
}

int main(const int argc, const char *argv[]) {
  timer_t timer = {0};
  clock_getres(CLOCK_THREAD_CPUTIME_ID, &timer.res);
  LOG_RES(timer);

  double (*foo)(int, double) = &multiply_foo;
  double (*bar)(int, double) = &multiply_bar;
  double (*funcs[])(int, double) = {foo, bar};
  size_t size = sizeof(funcs)/sizeof(funcs[0]);
  time_funcs(1000, 31.3, funcs, size);

  return 0;
}

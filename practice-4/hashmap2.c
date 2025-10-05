#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HASHMAP_SIZE 10000019
#define TABLE_RATIO 0.7

typedef struct {
  char* key;
  int value;
} entry_t;

int *get_entry_numbers(int table_size) {
  int *entry_numbers = malloc(table_size * sizeof(int));
  if (entry_numbers == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }
  
  entry_numbers[0] = rand() % 1000 + 1;
  for (int i = 1; i < table_size; i++) {
    entry_numbers[i] = entry_numbers[i - 1] + (rand() % 1000 + 1);
  }

  for (int i = 0; i < table_size; i++) {
    int j = rand() % table_size;
    int temp = entry_numbers[i];
    entry_numbers[i] = entry_numbers[j];
    entry_numbers[j] = temp;
  }

  return entry_numbers;
}

unsigned int hash1(const char* key, int hashmap_size) {
  unsigned int hash = 5381;
  for (int i = 0; key[i] != '\0'; i++) {
    hash = hash * 31 + key[i];
  }

  return hash % hashmap_size;
}

unsigned int hash2(const char* key, int hashmap_size) {
  unsigned int hash = 7;
  for (int i = 0; key[i] != '\0'; i++) {
    hash = hash * 17 + key[i];
  }

  return (hash % (hashmap_size - 1)) + 1;
}

typedef struct {
  entry_t **entries;
  unsigned int size;
  unsigned int count;
  unsigned long long colls;
} hashmap_t;

hashmap_t *hashmap_init(int size) {
  hashmap_t *map = malloc(sizeof(hashmap_t));
  if (map == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }

  map->entries = calloc(size, sizeof(entry_t));
  if (map->entries == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }

  map->size = size;
  map->count = 0;
  map->colls = 0;
  return map;
}

void hashmap_insert_linear(hashmap_t *map, const char* key, int value) {
  unsigned int index = hash1(key, map->size);
  unsigned int original_index = index;

  if (map->count >= map->size) {
    fprintf(stderr, "Linear: hashmap is full\n");
    return;
  }

  while (map->entries[index] != NULL) {
    if (strcmp(map->entries[index]->key, key) == 0)
      return;

    index = (index + 1) % map->size;
    map->colls++;

    if (index == original_index) {
      fprintf(stderr, "Linear: index looped back to original index\n");
      return;
    }
  }

  if (map->entries[index] == NULL) {
    map->entries[index] = malloc(sizeof(entry_t));
    map->entries[index]->key = strdup(key);
  }

  map->entries[index]->value = value;
  map->count++;
}

void hashmap_insert_double(hashmap_t *map, const char* key, int value) {
  unsigned int index = hash1(key, map->size);
  unsigned int step = hash2(key, map->size);
  unsigned int original_index = index;

  if (map->count >= map->size) {
    fprintf(stderr, "Double: hashmap is full\n");
    return;
  }

  while (map->entries[index] != NULL) {
    if (strcmp(map->entries[index]->key, key) == 0)
      return;

    index = (index + step) % map->size;
    map->colls++;

    if (index == original_index) {
      fprintf(stderr, "Double: index looped back to original index\n");
      return;
    }
  }

  if (map->entries[index] == NULL) {
    map->entries[index] = malloc(sizeof(entry_t));
    map->entries[index]->key = strdup(key);
  }

  map->entries[index]->value = value;
  map->count++;
}

void time_linear_probing(int *entry_numbers, int table_size, int hashmap_size) {
  hashmap_t *map = hashmap_init(hashmap_size);

  struct timespec start, end;
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);

  for (int i = 0; i < table_size; i++) {
    char key[20];
    sprintf(key, "key_%d", entry_numbers[i]);
    hashmap_insert_linear(map, key, entry_numbers[i]);
  }

  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
  double secs = end.tv_sec - start.tv_sec;
  double nsecs = end.tv_nsec - start.tv_nsec;
  if (nsecs < 0) {
    nsecs += 1000000000;
    secs -= 1;
  }

  const char *fmt = "Linear: colls: \t%llu \ttime: %d sec \t%d nsec\n";
  printf(fmt, map->colls, (int)secs, (int)nsecs);
  free(map->entries);
  free(map);
}

void time_double_probing(int *entry_numbers, int table_size, int hashmap_size) { 
  hashmap_t *map = hashmap_init(hashmap_size);

  struct timespec start, end;
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);

  for (int i = 0; i < table_size; i++) {
    char key[20];
    sprintf(key, "key_%d", entry_numbers[i]);
    hashmap_insert_double(map, key, entry_numbers[i]);
  }

  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
  double secs = end.tv_sec - start.tv_sec;
  double nsecs = end.tv_nsec - start.tv_nsec;
  if (nsecs < 0) {
    nsecs += 1000000000;
    secs -= 1;
  }

  const char *fmt = "Double: colls: \t%llu \ttime: %d sec \t%d nsec\n";
  printf(fmt, map->colls, (int)secs, (int)nsecs);
  free(map->entries);
  free(map);
}

int coprime(int a, int b) {
  while (b != 0) {
    int temp = b;
    b = a % b;
    a = temp;
  }

  return a == 1;
}

int main() {
  if(!coprime(HASHMAP_SIZE, HASHMAP_SIZE - 1)) {
    fprintf(stderr, "HASHMAP_SIZE and HASHMAP_SIZE - 1 are not coprime\n");
    return EXIT_FAILURE;
  }

  srand(time(NULL));
  int *entry_numbers = get_entry_numbers(HASHMAP_SIZE * TABLE_RATIO);

  time_linear_probing(entry_numbers, HASHMAP_SIZE * TABLE_RATIO, HASHMAP_SIZE);
  time_double_probing(entry_numbers, HASHMAP_SIZE * TABLE_RATIO, HASHMAP_SIZE);

  free(entry_numbers);
  return 0; 
}

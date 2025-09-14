#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <stdint.h>

#define THRESHOLD 32

int sum_test_sec = 0;
int sum_test_nsec = 0;

typedef struct {
  struct timespec res, start, end;
  int sec;
  long nsec;
} my_timer_t;

typedef struct {
  int *array;
  int size;
  my_timer_t timer;
} my_test_t;

static inline void swap(int *a, int *b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}

int partition(int *arr, int left, int right, int *lp) {
  if (left >= right) 
    return -1;

  int p = arr[left];
  int q = arr[right];
  if (p > q) 
    swap(&arr[left], &arr[right]);

  p = arr[left];
  q = arr[right];
  int i = left + 1, lt = left + 1, gt = right - 1;

  while (i <= gt) {
    if (arr[i] < p) {
      swap(&arr[i], &arr[lt]);
      lt++; i++;
    } else if (arr[i] > q) {
      swap(&arr[i], &arr[gt]);
      gt--;
    } else {
      i++;
    }
  }

  lt--; gt++;
  swap(&arr[left], &arr[lt]);
  swap(&arr[right], &arr[gt]);

  *lp = lt;
  return gt;
}

void quicksort(int *arr, int left, int right) {
  if (arr == NULL || left < 0 || right < 0 || left >= right)
    return;

  while (left < right) {
    int lp, rp;
    rp = partition(arr, left, right, &lp);
    if (rp == -1) 
      return;

    if (lp - left < right - rp) {
      quicksort(arr, left, lp - 1);
      left = rp + 1;
    } else {
      quicksort(arr, rp + 1, right);
      right = lp - 1;
    }
  }
}

void insertionsort(int *array, int left, int right) {
  if (array == NULL || left < 0 || right < 0 || left >= right)
    return;

  for (int i = left + 1; i <= right; i++) {
    int key = array[i];
    int j = i - 1;

    while (j >= 0 && array[j] > key) {
      array[j + 1] = array[j];
      j--;
    }

    array[j + 1] = key;
  }  
}

void hybridsort(int *array, int left, int right) {
  if (array == NULL || left < 0 || right < 0 || left >= right)
    return;

  while (left < right) {
    if (right - left + 1 <= THRESHOLD) {
      insertionsort(array, left, right);
      return;
    } 

    int lp, rp;
    rp = partition(array, left, right, &lp);
    if (rp == -1) 
      return;

    if (lp - left < right - rp) {
      hybridsort(array, left, lp - 1);
      left = rp + 1;
    } else {
      hybridsort(array, rp + 1, right);
      right = lp - 1;
    }
  }
}

int init_test(my_test_t *test, int size) {
  if (test == NULL || size <= 0) {
    printf("Error: NULL pointer or invalid size in init_test\n");
    return -1;
  }
  test->array = (int *)malloc(size * sizeof(int));
  if (test->array == NULL) {
    printf("Error: Memory allocation failed in init_test\n");
    return -1;
  }

  test->size = size;
  clock_getres(CLOCK_THREAD_CPUTIME_ID, &test->timer.res);
  return 0;
}

int generate_random_array(int *array, int size, int low, int high) {
  if (array == NULL || size <= 0 || low > high) {
    printf("Error: NULL pointer or invalid parameters in generate_random_array\n");
    return -1;
  }

  for (int i = 0; i < size; i++) {
    if (i % 2 == 0)
      array[i] = 42; 
    else
      array[i] = (rand() % (high - low + 1)) + low;
  }

  return 0;
}

int64_t sum_array(int *array, int size) {
  if (array == NULL || size <= 0) {
    printf("Error: NULL pointer or invalid size in sum_array\n");
    return -1;
  }

  int64_t sum = 0;
  for (int i = 0; i < size; i++) {
    sum += array[i];
  }

  return sum;
}

int is_sorted(int *array, int size) {
  if (array == NULL || size <= 0) {
    printf("Error: NULL pointer or invalid size in is_sorted\n");
    return -1;
  }

  for (int i = 0; i < size - 1; i++) {
    if (array[i] > array[i + 1]) {
      return -1;
    }
  }

  return 0;
}

int time_func(my_test_t *test, void (*func)(int *, int, int)) {
  if (test == NULL || func == NULL) {
    printf("Error: NULL pointer in time_func\n");
    return -1;
  }

  int64_t sum_before = sum_array(test->array, test->size);
  if (sum_before < 0) {
    printf("Error: sum_array failed in time_func\n");
    return -1;
  }

  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &test->timer.start);
  func(test->array, 0, test->size - 1);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &test->timer.end);
  test->timer.sec = test->timer.end.tv_sec - test->timer.start.tv_sec;
  test->timer.nsec = test->timer.end.tv_nsec - test->timer.start.tv_nsec;

  if (test->timer.nsec < 0) {
    test->timer.sec--;
    test->timer.nsec += 1000000000;
  }

  int64_t sum_after = sum_array(test->array, test->size);
  if (sum_before != sum_after) {
    printf("Error: Array modified in time_func\n");
    return -1;
  }

  if (!is_sorted(test->array, test->size)) {
    printf("Error: Array not sorted in time_func\n");
    return -1;
  }

  sum_test_sec += test->timer.sec;
  sum_test_nsec += test->timer.nsec;
  if (sum_test_nsec >= 1000000000) {
    sum_test_sec += sum_test_nsec / 1000000000;
    sum_test_nsec = sum_test_nsec % 1000000000;
  }

  return 0;
}

int twice_the_time(int sec1, long nsec1, int sec2, long nsec2) {
  if (sec2 > 2 * sec1)
    return 1;

  if (sec2 == 2 * sec1 && nsec2 >= 2 * nsec1)
    return 1;

  return 0;
}

int test_func(my_test_t *test, void (*func)(int *, int, int)) {
  if (test == NULL || func == NULL) {
    printf("Error: NULL pointer in test_func\n");
    return -1;
  }

  int tfr = time_func(test, func);
  if (tfr != 0) {
    printf("Error: time_func failed in test_func\n");
    return -1;
  }

  printf("(Unsorted) \tSize: %d \tTime: %d sec %ld nsec\n", test->size, test->timer.sec, test->timer.nsec);

  int sec = test->timer.sec;
  long nsec = test->timer.nsec;

  tfr = time_func(test, func);
  if (tfr != 0) {
    printf("Error: time_func failed in test_func (sorted)\n");
    return -1;
  }

  printf("(Sorted) \tSize: %d \tTime: %d sec %ld nsec\n", test->size, test->timer.sec, test->timer.nsec);
  if (twice_the_time(sec, nsec, test->timer.sec, test->timer.nsec)) {
    printf("Warning: Sorted array took more than twice the time of unsorted array\n");
  }

  return 0;
}

int main(const int argc, const char *argv[]) {
  if (argc < 3) {
    printf("Usage: %s <integer> <integer>\n", argv[0]);
    return 1;
  }

  for (int i = 0; argv[1][i] != '\0'; i++) {
    unsigned char c = argv[1][i];
    if (!isdigit(c)) {
      printf("Error: Argument 1 must be a positive integer.\n");
      return 1;
    }
  }

  for (int i = 0; argv[2][i] != '\0'; i++) {
    unsigned char c = argv[2][i];
    if (!isdigit(c)) {
      printf("Error: Argument 2 must be a positive integer.\n");
      return 1;
    }
  }

  int test_size = atoi(argv[1]);
  if (test_size <= 0) {
    printf("Error: Test size must be a positive integer.\n");
    return 1;
  }

  int test_num = atoi(argv[2]);
  if (test_num <= 0) {
    printf("Error: Test number must be a positive integer.\n");
    return 1;
  }

  my_test_t *tests = (my_test_t *)malloc(test_num * sizeof(my_test_t));
  if (tests == NULL) {
    printf("Error: Memory allocation failed.\n");
    return 1;
  }

  srand((unsigned int)time(NULL));
  for (int i = 0; i < test_num; i++) {
    if (init_test(&tests[i], test_size) != 0) {
      printf("Error: Test initialization failed.\n");
      free(tests);
      return 1;
    }

    if (generate_random_array(tests[i].array, tests[i].size, 0, 1000) != 0) {
      printf("Error: Random array generation failed.\n");
      free(tests);
      return 1;
    }
  }

  void (*qsort_func)(int *, int, int) = &quicksort;
  printf("Quick Sort Tests:\n");
  for (int i = 0; i < test_num; i++) {
    if (test_func(&tests[i], qsort_func) != 0) {
      printf("Error: Test %d failed.\n", i + 1);
      return 1;
    }
  }

  printf("\nSum time for %d tests of size %d: %d sec %d nsec\n\n", test_num, test_size, sum_test_sec, sum_test_nsec);

  for (int i = 0; i < test_num; i++) {
    if (generate_random_array(tests[i].array, tests[i].size, 0, 1000) != 0) {
      printf("Error: Random array generation failed.\n");
      free(tests);
      return 1;
    }
  }

  sum_test_sec = 0; 
  sum_test_nsec = 0;

  void (*hsort_func)(int *, int, int) = &hybridsort;
  printf("Hybrid Sort Tests:\n");
  for (int i = 0; i < test_num; i++) {
    if (test_func(&tests[i], hsort_func) != 0) {
      printf("Error: Test %d failed.\n", i + 1);
      return 1;
    }
  }

  printf("\nSum time for %d tests of size %d: %d sec %d nsec\n\n", test_num, test_size, sum_test_sec, sum_test_nsec);

  for (int i = 0; i < test_num; i++) {
    free(tests[i].array);
  }

  free(tests);
  return 0;
}

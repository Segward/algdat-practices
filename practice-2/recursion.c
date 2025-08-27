#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#define LOG_RES(T) \
  printf("CPU clock resolution: %lis %li ns\n", (T).res.tv_sec, (T).res.tv_nsec)

#define LOG_TIME(T, A, B, R) \
  printf("%d\t*\t%f\ttook\t%d s\t%li ns\trec:\t%d\n", (A), (B), (T).sec, (T).nsec, (R))

#define IS_EVEN(n) ((n) % 2 == 0)

typedef struct {
  struct timespec res;
  struct timespec start;
  struct timespec end;
  int sec;
  long nsec;
} my_timer_t;

double multiply_foo(int a, double b, int *rec_count) {
  if (a == 1)
    return b;

  *rec_count += 1;
  return b + multiply_foo(a - 1, b, rec_count);
}

double multiply_bar(int a, double b, int *rec_count) {
  if (a == 1)
    return b;
  
  *rec_count += 1;
  if (IS_EVEN(a))
    return multiply_bar(a/2, b + b, rec_count);

  return b + multiply_bar((a - 1)/2, b + b, rec_count);
}

int time_func(int a, double b, double (*func)(int, double, int*), my_timer_t *t, int *rec_count) {
  if (t == NULL || func == NULL)
    return -1;

  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t->start);
  double result = func(a, b, rec_count);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t->end);
  t->sec = t->end.tv_sec - t->start.tv_sec;
  t->nsec = t->end.tv_nsec - t->start.tv_nsec;
 
  if (t->nsec < 0) {
    t->sec -= 1;
    t->nsec += 1000000000;
  }

  return 0; 
}

typedef struct {
  int a;
  double b;
} my_mock_data_t;

int generate_mock_data(my_mock_data_t *data, const int size) {
  if (data == NULL || size <= 0)
    return -1;

  int a = 10;
  double b = (double)(rand() % 1000) / 10.0;

  for (int i = 0; i < size; i++) {
    data[i].a = a;
    data[i].b = b;
    a += 10;
  }

  return 0;
}

int main(const int argc, const char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <integer>\n", argv[0]);
    return 1;
  }

  for (int i = 0; argv[1][i] != '\0'; i++) {
    unsigned char c = argv[1][i];
    if (!isdigit(c)) {
      printf("Error: Argument must be a positive integer.\n");
      return 1;
    }
  }

  int mock_amount = atoi(argv[1]);
  if (mock_amount <= 0) {
    printf("Error: Mock amount must be a positive integer.\n");
    return 1;
  }

  my_timer_t timer = {0};
  clock_getres(CLOCK_THREAD_CPUTIME_ID, &timer.res);
  LOG_RES(timer);

  double (*foo)(int, double, int*) = &multiply_foo;
  double (*bar)(int, double, int*) = &multiply_bar;
 
  int example_a = 13;
  double example_b = 2.5;
  double expected = (double)(example_a) * example_b;
  int rec_count = 0;
  double foo_result = foo(example_a, example_b, &rec_count);
  if (fabs(foo_result - expected) > 1e-6) {
    printf("Error: multiply_foo failed. Expected %f, got %f\n", expected, foo_result);
    return 1;
  }

  printf("%d * %f = %f expected %f\n", example_a, example_b, foo_result, expected);

  example_a = 14;
  example_b = 10.1; 
  expected = (double)(example_a) * example_b;
  rec_count = 0;
  double bar_result = bar(example_a, example_b, &rec_count);
  if (fabs(bar_result - expected) > 1e-6) {
    printf("Error: multiply_bar failed. Expected %f, got %f\n", expected, bar_result);
    return 1;
  }

  printf("%d * %f = %f expected %f\n", example_a, example_b, bar_result, expected);
 
  my_mock_data_t *data = malloc(sizeof(my_mock_data_t) * mock_amount);
  if (data == NULL) {
    printf("Error: Memory allocation failed.\n");
    return 1;
  }

  srand(time(NULL));
  if (generate_mock_data(data, mock_amount) != 0) {
    printf("Error: Failed to generate mock data.\n");
    free(data);
    return 1;
  }

  int foo_rec_count = 0;
  int bar_rec_count = 0;

  printf("\nUsing multiply_foo:\n");
  for (int i = 0; i < mock_amount; i++) {
    if (time_func(data[i].a, data[i].b, foo, &timer, &foo_rec_count) != 0) {
      printf("Error: Timing function failed.\n");
      free(data);
      return 1;
    }
    LOG_TIME(timer, data[i].a, data[i].b, foo_rec_count);
    foo_rec_count = 0;
  } 

  printf("\nUsing multiply_bar:\n");
  for (int i = 0; i < mock_amount; i++) {
    if (time_func(data[i].a, data[i].b, bar, &timer, &bar_rec_count) != 0) {
      printf("Error: Timing function failed.\n");
      free(data);
      return 1;
    }
    LOG_TIME(timer, data[i].a, data[i].b, bar_rec_count);
    bar_rec_count = 0;
  }



  return 0;
}

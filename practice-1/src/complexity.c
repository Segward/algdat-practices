#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define IS_NOT_DIGIT(c) \
  ((c) < '0' || (c) > '9')

//  this is o(n) because it iterates from 0 to n-1 once 
int o(int n) {
  int t = 0;
  for (int i = 0; i < n; i++) {
    t += i;
  }
  return t;
}

//  this is o(n^2) because it iterates from 0 to n-1 \
    for each i in 0 to n-1
int o2(int n) {
  int t = 0;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      t += i + j;
    }
  }
  return t;
}

// get the number of iterations from argument
int main(int argc, const char **argv) {
  if (argc < 2) {
    printf("Usage: %s <number>\n", argv[0]);
    return 1;
  }

  for (int i = 0; argv[1][i] != '\0'; i++) {
    char c = argv[1][i];
    if (IS_NOT_DIGIT(c)) {
      printf("Error: Argument must be a number.\n");
      return 1;
    }  
  }

  int n = atoi(argv[1]);
  if (n < 1) {
    printf("Error: Number must be non-negative.\n");
    return 1;
  }
 
  // use clock to measure the time taken by each function 
  clock_t start = clock();
  int result = o(n);
  clock_t end = clock();
  clock_t diff = end - start;
  printf("Cycle count for o(n): %ld\n", diff);

  start = clock();  
  result = o2(n);
  end = clock();
  diff = end - start;
  printf("Cycle count for o(n^2): %ld\n", diff);

  return 0;
}

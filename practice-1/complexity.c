#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

// some macros for logging
#define LOG_RES(T) \
  printf("CPU clock resolution: %lis %li ns\n", (T).res.tv_sec, (T).res.tv_nsec)

#define LOG_TRANSACTION(T) \
  printf("Buy: %d, Sell: %d, Profit: %d\n", (T).buy, (T).sell, (T).profit)

#define LOG_TIMER(T, S) \
  printf("Time taken for \t%zu size: \t%d sec \t%ld nsec\n", S, (T).sec, (T).nsec)

// struct to hold changes in values for stock prices
typedef struct {
  int *values;
  size_t size;
} my_changes_t;

// struct to hold transaction details
typedef struct {
  int buy;
  int sell;
  int profit;
  my_changes_t *changes;
} my_transaction_t; 

// my struct to hold timing details
typedef struct {
  struct timespec res;
  struct timespec start;
  struct timespec end;
  int sec;
  long nsec;
} my_timer_t;

// struct to hold mock results
typedef struct {
  my_timer_t timer;
  size_t size;
} my_mock_result_t;

// generates random changes range from -10 to 10
void generate_changes(my_changes_t *changes, size_t size) {
  changes->size = size;
  changes->values = malloc(size * sizeof(int));
  if (changes->values == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }

  for (size_t i = 0; i < size; i++) {
    changes->values[i] = rand() % 21 - 10;
  }
}

// computes the maximum profit from the changes in stock prices. \
  this is the actual algorithm for the practice problem
void compute_profit(my_transaction_t *transaction) {
  int min_value = 0, min_index = 0;
  int current_value = 0, max_profit = 0;

  for (size_t i = 0; i < transaction->changes->size; i++) {
    current_value += transaction->changes->values[i];

    // update the minimum value when we find a new minimum
    if (current_value < min_value) {
      min_value = current_value;
      min_index = i;
    }

    // update the maximum profit if the current profit is greater \
      and set the buy and sell indices
    int profit = current_value - min_value;
    if (profit > max_profit && i > min_index) {
      max_profit = profit;
      transaction->buy = min_index;
      transaction->sell = i;
    }
  }

  transaction->profit = max_profit;
}

// times the compute_profit function
void time_compute_profit(my_transaction_t *transaction, my_timer_t *timer) {
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &timer->start);
  compute_profit(transaction);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &timer->end);
  timer->sec = timer->end.tv_sec - timer->start.tv_sec;
  timer->nsec = timer->end.tv_nsec - timer->start.tv_nsec;
  if (timer->nsec < 0) {
    timer->sec -= 1;
    timer->nsec += 1000000000L;
  }
}

// main function. we take one argument which is the \
  amount <integer> of mock data to test.
int main(const int argc, const char **argv) {
  if (argc < 2) {
    printf("Usage: %s <integer>\n", argv[0]);
    return 1;
  }

  // check if the argument only contains digits
  for (int i = 0; argv[1][i] != '\0'; i++) {
    unsigned char c = argv[1][i];
    if (!isdigit(c)) {
      printf("Error: Argument must be a positive integer.\n");
      return 1;
    }
  }

  // convert the argument to an integer
  int mock_amount = atoi(argv[1]);
  if (mock_amount <= 0) {
    printf("Error: Mock amount must be a positive integer.\n");
    return 1;
  }

  // seed the random number generator
  srand(time(NULL));

  // this is the example data given in the practice \
    problem description
  int example[] = {-1, 3, -9, 2, 2, -1, 2, -1, -5};
  size_t size = sizeof(example) / sizeof(example[0]); 

  my_changes_t changes;
  changes.size = size;
  changes.values = example;

  my_transaction_t transaction;
  transaction.changes = &changes;
  transaction.buy = -1;
  transaction.sell = -1;
  transaction.profit = 0;

  // log the CPU clock resolution
  my_timer_t timer = {0};
  clock_getres(CLOCK_MONOTONIC, &timer.res);
  LOG_RES(timer);

  // time the compute_profit function with the example data
  time_compute_profit(&transaction, &timer);
  LOG_TRANSACTION(transaction);
  LOG_TIMER(timer, changes.size);

  // array to hold the mock results
  my_mock_result_t results[mock_amount];

  // generate mock changes and time the compute_profit function
  size_t mock_size = 10;
  for (int i = 0; i < mock_amount; i++) {
    my_changes_t mock_changes;
    generate_changes(&mock_changes, mock_size);

    my_transaction_t transaction; 
    transaction.changes = &mock_changes;
    transaction.buy = -1;
    transaction.sell = -1;
    transaction.profit = 0;

    // time the compute_profit function and store the results
    my_timer_t mock_timer = {0};
    clock_getres(CLOCK_MONOTONIC, &mock_timer.res);
    time_compute_profit(&transaction, &mock_timer);
    results[i].timer = mock_timer;
    results[i].size = mock_size;

    // free the allocated memory since it's on the heap \
      and we don't need it anymore
    free(mock_changes.values);
  
    // double the size of the mock data to look for patterns \
      in the timing
    mock_size *= 2;
  }

  // print the results from the mock data
  for (int i = 0; i < mock_amount; i++) {
    LOG_TIMER(results[i].timer, results[i].size);
  }

  return 0;
}

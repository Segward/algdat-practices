#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

#define LOG_TIMER(T, S) \
  printf("Time taken: %ld cycles for %zu size data\n", \
    (T).cycle_count, (S)); \

#define LOG_TRANSACTION(T) \
  printf("Transaction: Buy at %d, Sell at %d, Profit %d\n", \
    (T).buy, (T).sell, (T).profit);

typedef struct {
  int *values;
  size_t size;
} changes_t;

typedef struct {
  int buy;
  int sell;
  int profit;
  changes_t *changes;
} transaction_t; 

typedef struct {
  clock_t start;
  clock_t end;
  clock_t cycle_count;
} timer_t;

void generate_changes(changes_t *changes, size_t size) {
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

void compute_profit(transaction_t *transaction) {
  int min_value = 0, min_index = 0;
  int current_value = 0, max_profit = 0;

  for (size_t i = 0; i < transaction->changes->size; i++) {
    current_value += transaction->changes->values[i];

    if (current_value < min_value) {
      min_value = current_value;
      min_index = i;
    }

    int profit = current_value - min_value;
    if (profit > max_profit && i > min_index) {
      max_profit = profit;
      transaction->buy = min_index;
      transaction->sell = i;
    }
  }

  transaction->profit = max_profit;
}

void time_compute_profit(transaction_t *transaction, timer_t *timer) {
  timer->start = clock();
  compute_profit(transaction);
  timer->end = clock();
  timer->cycle_count = timer->end - timer->start;
}

int main(int argc, const char **argv) {
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

  srand(time(NULL));
  printf("CLOCKS_PER_SEC = %ld\n", CLOCKS_PER_SEC);

  int example[] = {-1, 3, -9, 2, 2, -1, 2, -1, -5};
  size_t size = sizeof(example) / sizeof(example[0]); 

  changes_t changes = {0};
  changes.size = size;
  changes.values = example;

  transaction_t transaction = {0};
  transaction.changes = &changes;
  transaction.buy = -1;
  transaction.sell = -1;
  transaction.profit = 0;

  timer_t timer = {0};
  time_compute_profit(&transaction, &timer);
  LOG_TRANSACTION(transaction);
  LOG_TIMER(timer, size);

  size_t mock_size = 10;
  for (int i = 0; i < mock_amount; i++) {
    changes_t mock_changes;
    generate_changes(&mock_changes, mock_size);

    transaction.changes = &mock_changes;
    transaction.buy = -1;
    transaction.sell = -1;
    transaction.profit = 0;

    time_compute_profit(&transaction, &timer);
    LOG_TIMER(timer, mock_size);

    free(mock_changes.values);
    mock_size *= 2;
  }

  return 0;
}

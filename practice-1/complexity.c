#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int profit(int *changes, int size, int *buy, int *sell) {
  int min_value = 0;
  int min_index = 0;
  int current_value = 0;
  int max_profit = 0;

  // iterate through changes
  for (int i = 0; i < size; i++) {
    current_value += changes[i];
    
    //  update minimum value and index \
        if the current value is less than the minimum value
    if (current_value < min_value) {
      min_value = current_value;
      min_index = i;
    }

    //  update max profit if the current profit \
        is greater than the maximum profit
    int profit = current_value - min_value;
    if (profit > max_profit && i > min_index) { 
      max_profit = profit;
      *buy = min_index;
      *sell = i;
    }
  }

  return max_profit;
}

int main(int argc, const char **argv) {
  int changes[] = {-1, 3, -9, 2, 2, -1, 2, -1, -5};
  int size = sizeof(changes) / sizeof(changes[0]); 
  int buy = 0, sell = 0;

  // use clock to measure the time taken by each function 
  clock_t start = clock();
  int max_profit = profit(changes, size, &buy, &sell);
  clock_t end = clock();
  clock_t diff = end - start;
  printf("Cycle count for foo: %ld\n", diff);

  //  since the index given starts from 0, we add 1 to buy and sell \
      to get the actual days rather than indices
  int buy_day = buy + 1;
  int sell_day = sell + 1;
  printf("Max profit: %d, Buy on day %d, Sell on day %d\n", 
    max_profit, buy_day, sell_day);

  return 0;
}

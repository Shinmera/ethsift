#ifndef FLOP_COUNTERS_H
#define FLOP_COUNTERS_H

/// Methods and variables for counting flops
#ifdef IS_COUNTING
  size_t add_count, mult_count, mem_count, div_count;
  
  static int reset_counters() {
    add_count = 0;
    mult_count = 0;
    mem_count = 0;
    div_count = 0;
    return 1;
  }

  static int inc_counters(int add_amount, int mults_amount, int mem_amount, int div_amount) {
    add_count += add_amount;
    mult_count += mults_amount;
    mem_count += mem_amount;
    div_count += div_amount;
    return 1;
  }

  // Macro to count adds/subs
  #define inc_adds(AMOUNT) inc_counters(AMOUNT, 0, 0, 0);
  #define inc_mults(AMOUNT) inc_counters(0, AMOUNT, 0, 0);
  #define inc_mem(AMOUNT) inc_counters(0, 0, AMOUNT, 0);
  #define inc_div(AMOUNT) inc_counters(0, 0, 0, AMOUNT);

#else
  #define inc_adds(AMOUNT)
  #define inc_mults(AMOUNT)
  #define inc_mem(AMOUNT)
  #define inc_div(AMOUNT)
#endif

#endif

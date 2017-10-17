// SAFE
typedef unsigned int __uint32_t;
typedef __uint32_t uint32_t;

uint32_t E(uint32_t);
uint32_t abs_rtos_get_time(void);

static inline __uint32_t get_stuff_thing_index(const __uint32_t thing_index) {
  const __uint32_t result = thing_index % 19U;
  return result;
}

void start(const __uint32_t thing_index,
           __uint32_t* const p_foo_stuff_time,
           __uint32_t* const p_start_time,
           const __uint32_t current_time);

void handle(uint32_t* const p_thing_index,
            uint32_t* const p_start_time,
            uint32_t* const p_foo_stuff_time) {
  if (*p_thing_index < E(3U * 19U)) {
    const uint32_t thing_index = get_stuff_thing_index(*p_thing_index);
    const uint32_t current_time = abs_rtos_get_time();

    if (0 == *p_start_time) {
      start(*p_thing_index, p_foo_stuff_time, p_start_time, current_time);
    }
  }
}

// BEGIN MODIFICATION
// Need to call handle with three initialized pointers.
// Otherwise, no assumption about the pointer addresses can be made so
// pointer dereference such as "*p_thing_index" will raise warnings.

extern uint32_t NONDET(); // this is used to initialize variables
                          // assuming that it can take any possible value.

int main() {
  // Here arbitrary values to these three
  uint32_t p_thing_index = NONDET();
  uint32_t p_start_time = NONDET();
  uint32_t p_foo_stuff_time = NONDET();

  handle(&p_thing_index, &p_start_time, &p_foo_stuff_time);
  return 42;
}
// END MODIFICATION

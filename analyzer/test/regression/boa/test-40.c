// SAFE
#include <stdlib.h>

typedef __signed char __int8_t;
typedef unsigned char __uint8_t;
typedef unsigned short int __uint16_t;
typedef unsigned int __uint32_t;
typedef __uint32_t uint32_t;

typedef struct {
  __uint32_t error_count;
} state_data_t;

typedef struct {
  __uint16_t foo_sample_size;
  __uint16_t foo_means[4];
  __uint16_t foo_sds[4];
  __uint16_t bar_sample_size;
  __uint16_t bar_means[32];
  __uint16_t bar_sds[32];
} sample_data_t;

typedef struct {
  __uint32_t alt_pressure;
  __int8_t alt_foo;
  __uint8_t hum_relative_humidity;
  __int8_t hum_foo;
} stuff_data_t;

typedef enum { ERR_NO_ERROR = 0, ERR_COUNT } err_t;

typedef struct {
  state_data_t state;
  sample_data_t sample;
  stuff_data_t stuff;
} response_packet_t;

typedef enum {
  SELECT_NONE = 0,
  SELECT_1,
  SELECT_2,
  SELECT_3,
} command_t;

static const command_t EXPERIMENT[3U] = {SELECT_2, SELECT_3, SELECT_1};

// BEGIN MODIFICATION
extern __uint8_t NONDET();
// I replaced the given get_index function because when it is called
// below its formal parameter is always uninitialized so a warning
// must be raised. The version I have is one that only assumes that
// the return value must be between 0 and 2 because it is used as index
// in EXPERIMENT whose size is 3.
static inline __uint32_t get_index(const __uint32_t thing_index) {
  __uint32_t number = NONDET();
  if (number <= 2)
    return number;
  else
    exit(0);
}
/* static inline __uint32_t get_index(const __uint32_t thing_index) */
/* { */
/*     const __uint32_t result = thing_index / 19U; */
/*     return result; */
/* } */
// END MODIFICATION
err_t send(const command_t command, response_packet_t* const p_response_packet);

void start(const __uint32_t thing_index,
           __uint32_t* const p_bar_experiment_time,
           __uint32_t* const p_start_time,
           const __uint32_t current_time) {
  response_packet_t response;
  const uint32_t index = get_index(thing_index);

  const command_t select_command = EXPERIMENT[index];

  const err_t err = send(select_command, &response);
}

int main(void) {
  const __uint32_t foo = 0;
  start(foo, &foo, &foo, foo);
}

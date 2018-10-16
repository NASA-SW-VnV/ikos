// SAFE
#include <stdlib.h>

typedef unsigned char __uint8_t;
typedef unsigned int __uint32_t;
typedef __uint32_t uint32_t;

typedef enum {
  FOO_BAR_ERR_NO_ERROR = 0,
  FOO_BAR_ERR_PEX,
  FOO_BAR_ERR_NULL,
  FOO_BAR_ERR_OS_ERR,
  FOO_BAR_ERR_BAD_SIZE,
  FOO_BAR_ERR_BAR_CREATE_FAIL
} foo_bar_err_t;

typedef struct {
  uint32_t last_comm_time;
  uint32_t dep_time;
  uint32_t start_timeout_sec;
  int valid;
} start_t;

typedef enum {
  COMMAND_OPCODE_RESET = 3,
  COMMAND_OPCODE_START_EXPERIMENT = 9,
  COMMAND_OPCODE_CANCEL_EXPERIMENT = 22,
  COMMAND_OPCODE_SET_EXPERIMENT = 24
} code_t;

typedef struct {
  __uint32_t command_id;
  __uint32_t execution_time;
  code_t command_opcode;
  __uint8_t command_parameters[115U];
} types_command_packet_t;

extern void* barset(void*, int, size_t);

int do_something(types_command_packet_t* const p_command_packet);
extern start_t fetch_values(void);
// void * foo_bar_malloc (const size_t size, foo_bar_err_t * const perr);
// BEGIN MODIFICATION
// We had the prototype for foo_bar_malloc but not its
// implementation. Without it, we cannot say anything about the void *
// returned by the function.
void* foo_bar_malloc(const size_t size, foo_bar_err_t* const perr) {
  return (void*)malloc(size);
}
// END MODIFICATION

foo_bar_err_t foo_bar_free(void* const block);

void start(void) {
  const start_t values = fetch_values();
  if ((values.valid) && (values.dep_time > values.last_comm_time)) {
    foo_bar_err_t bar_err;
    types_command_packet_t* const command_packet =
        foo_bar_malloc(sizeof(types_command_packet_t), &bar_err);

    if (command_packet) {
      command_packet->command_id = 1;
      command_packet->execution_time =
          values.dep_time + values.start_timeout_sec;
      command_packet->command_opcode = COMMAND_OPCODE_START_EXPERIMENT;
      barset(command_packet->command_parameters, 0, 115U);
      command_packet->command_parameters[0] = 0xBC;
      int const done = do_something(command_packet);
      if (!done) {
        bar_err = foo_bar_free(command_packet);
      }
    }
  }
}

int main(void) {
  start();
}

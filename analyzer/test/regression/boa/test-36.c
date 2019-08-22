// SAFE
typedef unsigned char __uint8_t;
typedef unsigned int __uint32_t;
typedef long unsigned int size_t;

typedef void (*entry_t)(void*);
typedef struct {
  entry_t entry_point;
  __uint8_t priority;
  __uint32_t* stack;
  __uint32_t stack_size;
  const char* name;
} foo_bar_ts_t;

typedef const foo_bar_ts_t* (*ts_getter_t)(void);

void foo_bar_create_task(const foo_bar_ts_t* const p_ts);

const foo_bar_ts_t* get_shell_ts(void);
const foo_bar_ts_t* get_controller_ts(void);

static const ts_getter_t TS_GETTERS[] = {
    get_shell_ts,
    get_controller_ts,
};

const foo_bar_ts_t* start_get_ts(void);
static void create_application_tasks(void);

int main(int argc, char** argv) {
  create_application_tasks();
  return 0;
}

static void create_application_tasks(void) {
  const size_t n = sizeof(TS_GETTERS) / sizeof(ts_getter_t);
  size_t getter_index;
  for (getter_index = 0; getter_index < n; getter_index++) {
    const ts_getter_t getter_fn = TS_GETTERS[getter_index];
    const foo_bar_ts_t* const p_ts = getter_fn();
    foo_bar_create_task(p_ts);
  }
}

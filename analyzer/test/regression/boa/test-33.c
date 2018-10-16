// SAFE
#include <stdlib.h>

typedef enum {

  ABS_RR_ERR_NO_ERROR = 0,
  ABS_RR_ERR_COUNT
} foo_bar_err_t;

typedef unsigned int __uint32_t;
typedef void* foo_bar_t;

foo_bar_err_t test_seize_resource(const foo_bar_t resource);
foo_bar_err_t test_relinquish_resource(const foo_bar_t resource);

foo_bar_t test_get_resource(__uint32_t);
static struct { __uint32_t sampling_interval_seconds; } g = {0};

////
// Note that test_get_resource is not defined!
////

static inline foo_bar_t get_resource(void) {
  static foo_bar_t resource = 0;
  if (!resource) {
    resource = test_get_resource(12U);
  }
  return resource;
}

int main(int argc, char** argv) {
  const foo_bar_err_t test_err = test_seize_resource(get_resource());
  if (ABS_RR_ERR_NO_ERROR == test_err) {
    g.sampling_interval_seconds = argc;
    test_relinquish_resource(get_resource());
  }
  return 0;
}

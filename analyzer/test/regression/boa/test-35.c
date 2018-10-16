// SAFE
#include <string.h>

typedef unsigned int __uint32_t;

typedef enum {
  RESET_BOR,
  RESET_CMR,
  RESET_MCLR,
  RESET_POR,
  RESET_SWR,
  RESET_WDTR,
  RESET_COUNT
} types_reset_t;

typedef enum { ERROR = 0, INFO, TRACE, LOG_LEVEL_COUNT } util_log_level_t;

void util_log(util_log_level_t const level,
              const char* const file,
              __uint32_t const line,
              const char* const format,
              ...);

types_reset_t bsp_reset_get();

int main(int argc, char** argv) {
  const types_reset_t reset = bsp_reset_get();

  static const char* const STRING_MAP[] = {"brown-out",
                                           "configuration mismatch",
                                           "master clear",
                                           "power on",
                                           "software",
                                           "watchdog timeout"};
  const char* const reset_string =

      (((types_reset_t)0 <= reset) && (reset < RESET_COUNT)) ? STRING_MAP[reset]
                                                             : "unknown";

  char msg[55];
  strcpy(msg, "The cause of the last reset was ");
  strcat(msg, reset_string);
  { util_log(INFO, "../../src/init/init.c", 356, msg, ""); };

  return 0;
}

/* va_copy example */
#include <stdarg.h> /* va_list, va_start, va_copy, va_arg, va_end */
#include <stdio.h>  /* printf, vprintf*/
#include <stdlib.h> /* malloc */
#include <string.h> /* strlen, strcat */

/* print ints until a zero is found: */
void PrintInts(int first, ...) {
  char* buffer;
  const char* format = "[%d] ";
  int count = 0;
  int val = first;
  va_list vl, vl_count;
  va_start(vl, first);

  /* count number of arguments: */
  va_copy(vl_count, vl);
  while (val != 0) {
    val = va_arg(vl_count, int);
    ++count;
  }
  va_end(vl_count);

  /* allocate storage for format string: */
  buffer = (char*)malloc(strlen(format) * count + 1);
  buffer[0] = '\0';

  /* generate format string: */
  for (; count > 0; --count) {
    strcat(buffer, format);
  }

  /* print integers: */
  printf(format, first);
  vprintf(buffer, vl);

  va_end(vl);
}

int main() {
  PrintInts(10, 20, 30, 40, 50, 0);
  return 0;
}

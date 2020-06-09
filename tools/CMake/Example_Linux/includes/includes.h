#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define   sl_free    free
#define   sl_malloc  malloc

#define  debug_info(x,y...)    printf(x,##y) //printf(x,##__VA_ARGS__)
#define  debug_message(x,y...)    printf(x,##y) //printf(x,##__VA_ARGS__)
#define  debug_error(x,y...)    printf(x,##y) //printf(x,##__VA_ARGS__)


typedef uint8_t bool_t;
typedef int32_t err_t;


#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "platform.h"

#define ARRAY_COUNT(x) (sizeof(x)/sizeof(*(x)))
#define MIN(x,y)  ( ((x) > (y)) ? (y) : (x) )
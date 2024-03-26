#ifndef __HELPER_H__
#define __HELPER_H__

#define SATURATE(x, a, b) ((x) < (a) ? (a) : (x) > (b) ? (b) : (x))
#define NORMALIZE(x, a, b) (((double) (x) - (a)) / ((double) (b) - (a)))
#define MAP(x, a, b) ((a) + x * ((b) - (a)))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define SIGN(x) ((x) > 0 ? 1 : (x) < 0 ? -1 : 0)
#define ARR_SIZE(x) (sizeof((x)) / sizeof((x)[0]))

#endif

#ifndef __HELPER_H__
#define __HELPER_H__

#define SATURATE(x, a, b) ((x) < (a) ? (a) : (x) > (b) ? (b) : (x))
#define NORMALIZE(x, a, b) (((double) (x) - (a)) / ((double) (b) - (a)))
#define MAP(x, a, b) ((a) + x * ((b) - (a)))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))

#endif

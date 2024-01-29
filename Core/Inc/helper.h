#ifndef __HELPER_H__
#define __HELPER_H__

#define SATURATE(x, a, b) ((x) < (a) ? (a) : (x) > (b) ? (b) : (x))
#define NORMALIZE(x, a, b) (((x) - (a)) / ((b) - (a)))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#endif

#define SATURATE(x, a, b) ((x) < (a) ? (a) : (x) > (b) ? (b) : (x))
#define NORMALIZE(x, a, b) (((x) - (a)) / ((b) - (a)))

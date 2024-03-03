#define __DEBUG__
#ifdef __DEBUG__
#include <cstdio>
#include <cassert>
#define LOG(format, ...) printf(format, __VA_ARGS__)
#define CHECK(cond) assert(cond)
#elif
#define LOG(format, ...) ;
#define CHECK(cond) ;
#endif
#pragma once


#ifdef _MSC_VER
#include <intrin.h>
#endif

inline size_t Seed() {
#ifdef _MSC_VER
	return (size_t)__rdtsc();
#else
	return std::clock();
#endif
}


enum eAction {
	UP = 0,
	DOWN = 1,
	LEFT = 2,
	RIGHT = 3,
};


#pragma once


#ifdef _MSC_VER
#include <intrin.h>
#endif

/// Generates a seed to be used to initialize random number engines.
/// If compiled with MSVC, it reads the cpu's timestamp counter, which
/// is pretty much perfect random for our uses.
inline size_t Seed() {
#ifdef _MSC_VER
	return (size_t)__rdtsc();
#else
	return std::clock();
#endif
}

/// The available actions in the 'mines' game.
enum eAction {
	UP = 0,
	DOWN = 1,
	LEFT = 2,
	RIGHT = 3,
};


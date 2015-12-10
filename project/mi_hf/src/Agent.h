#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <random>
#include "Util.h"

class Game;
class Map;

using real = double;

template <class T>
struct DummyF {
	uint32_t pad1[4];
	T vec[4];
	uint32_t pad2[4];

	T& operator[](size_t idx) {
		T& v = vec[idx];
		CheckPads();
		return v;
	}
	T operator[](size_t idx) const {
		T v = vec[idx];
		CheckPads();
		return v;
	}
	T* begin() {
		return vec;
	}
	T* end() {
		return vec + 4;
	}
	const T* begin() const {
		return vec;
	}
	const T* end() const {
		return vec + 4;
	}

	DummyF() {
		for (auto& v : pad1) {
			v = 0xDEADBEEF;
		}
		for (auto& v : pad2) {
			v = 0xDEADBEEF;
		}
	}

	void CheckPads() const {
		for (auto& v : pad1) {
			if (v != 0xDEADBEEF) {
				__debugbreak();
			}
		}
		for (auto& v : pad2) {
			if (v != 0xDEADBEEF) {
				__debugbreak();
			}
		}
	}
};

class Agent {
public:
	Agent();
	~Agent() = default;


	void Step();
	void SetGame(Game* game);
	void Reset();
	void StartEpisode();
	float EndEpisode();

	float GetQ(int x, int y, eAction action) const;
	float GetQMax(int x, int y) const;
private:
	void SelectNextStep(int x, int y, const Map& map, real& reward, eAction& action);

	std::vector</*std::array<float, 4>*/DummyF<real>> Q;
	std::vector</*std::array<int, 4>*/DummyF<int>> N;
	Game* currentGame;
	eAction lastAction;
	real totalReward;

	std::mt19937 rne;
	std::uniform_real_distribution<real> rng_roll;
	std::uniform_int_distribution<int> rng_action;
};

#pragma once

#include <random>
#include "Util.h"


class Map;


class Game {
public:
	Game();
	~Game() = default;


	void PerformAction(eAction action);
	float GetCurrentReward() const;
	int GetCurrentX() const;
	int GetCurrentY() const;

	void NewGame();
	bool Ended();

	void SetMap(Map* map);
	Map* GetMap() { return map; }
	const Map* GetMap() const { return map; }
private:
	Map* map;
	int posx, posy;
	bool ended;
	std::mt19937 rne;
};
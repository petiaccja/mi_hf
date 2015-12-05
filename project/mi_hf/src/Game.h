#pragma once

#include <random>


class Map;


enum eAction {
	UP = 0,
	DOWN = 1,
	LEFT = 2,
	RIGHT = 3,
};


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
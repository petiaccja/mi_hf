#pragma once

#include <vector>

class Game;



class Agent {
private:
	struct Record {
		int posx, posy; // s
		float utility[4];
		int count[4];
	};

public:
	Agent();
	~Agent() = default;


	void Step();
	void SetGame(Game* game);
	void Reset();

private:
	std::vector<Record> Q;
	Game* currentGame;
};

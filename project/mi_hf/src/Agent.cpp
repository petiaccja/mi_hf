#include "Agent.h"
#include "Game.h"
#include "Map.h"

Agent::Agent() {
	currentGame = nullptr;
}

void Agent::Step() {
	// TODO: determine next step by the pi strategy
	// perform next step
	currentGame->PerformAction(UP);

	// perception of the environment
	int x = currentGame->GetCurrentX();
	int y = currentGame->GetCurrentY();
	float r = currentGame->GetCurrentReward();

	// update Q table

}

void Agent::Reset() {
	for (auto& q : Q) {
		for (auto& v : q.count) {
			v = 0;
		}
		for (auto& v : q.utility) {
			v = 0;
		}
	}
}


void Agent::SetGame(Game* game) {
	currentGame = game;
	if (game->GetMap()) {
		int w = game->GetMap()->GetWidth();
		int h = game->GetMap()->GetHeight();
		Q.resize(w*h);
	}
	Reset();
}
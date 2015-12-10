#include "Agent.h"
#include "Game.h"
#include "Map.h"

#include <algorithm>
#include <cassert>


Agent::Agent() :
	rng_roll(0.0f, 1.0f),
	rng_action(0, 3),
	rne(Seed())
{
	currentGame = nullptr;
}


eAction Agent::SelectNextStep(int x, int y) {
	real roll = rng_roll(rne);
	eAction action;
	// normal behaviour
	float utility = -std::numeric_limits<real>::infinity();
	if (roll > 0.03) {		
		for (int i = 0; i < 4; i++) {
			float value = Q(x, y, (eAction)i);
			if (utility < value) {
				utility = value;
				action = (eAction)i;
			}			
		}


		if (utility == -std::numeric_limits<real>::infinity()) {
			__debugbreak();
		}
		assert(utility != -std::numeric_limits<real>::infinity());
	}
	// random exploration
	else {
		action = (eAction)rng_action(rne);
	}
	return action;
}


void Agent::Step() {
	if (!currentGame || !currentGame->GetMap()) {
		return;
	}

	// variables
	int x = currentGame->GetCurrentX();
	int y = currentGame->GetCurrentY();
	int newx, newy;
	real Qold;
	real Qmax;
	eAction action;
	real reward;

	// select action by strategy
	action = SelectNextStep(x, y);

	Qold = GetQ(x, y, action);

	// perform action
	bool isOver = currentGame->PerformAction(action);

	// perceive the environment
	reward = currentGame->GetCurrentReward();
	newx = currentGame->GetCurrentX();
	newy = currentGame->GetCurrentY();

	// update Q accordingly
	Qmax = GetQMax(newx, newy);

	if (!isOver) {
		Q(x, y, action) = Qold + alpha*(reward + gamma*Qmax - Qold);
	}
	else {
		for (int i = 0; i < 4; i++) {
			Q(newx, newy, (eAction)i) = reward;
		}
		Q(x, y, action) = Qold + alpha*(reward - Qold);
	}

	// log reward just for fun
	totalReward += reward;
}

void Agent::Reset() {
	for (auto& q : Q_) {
		for (auto& v : q) {
			v = 0;
		}
	}
	for (auto& n : N_) {
		for (auto& v : n) {
			v = 0;
		}
	}
}


void Agent::StartEpisode() {
	totalReward = 0;
}


float Agent::EndEpisode() {
	return totalReward;
}


void Agent::SetGame(Game* game) {
	currentGame = game;
	if (game->GetMap()) {
		width = game->GetMap()->GetWidth();
		height = game->GetMap()->GetHeight();
		Q_.resize(width*height);
		N_.resize(width*height);
	}
	Reset();
}

float& Agent::Q(int x, int y, eAction action) {
	assert(0 <= x && x < width && 0 <= y && y < height);
	return Q_[y*width + x][action];
}

int& Agent::N(int x, int y, eAction action) {
	assert(0 <= x && x < width && 0 <= y && y < height);
	return N_[y*width + x][action];
}

float Agent::GetQ(int x, int y, eAction action) const {
	assert(0 <= x && x < width && 0 <= y && y < height);
	return Q_[y*width + x][action];
}

float Agent::GetQMax(int x, int y) const {
	assert(0 <= x && x < width && 0 <= y && y < height);
	return *std::max_element(Q_[y*width + x].begin(), Q_[y*width + x].end());
}
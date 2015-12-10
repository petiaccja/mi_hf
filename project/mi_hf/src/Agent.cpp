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

void Agent::SelectNextStep(int x, int y, const Map& map, real& reward, eAction& action) {
	real roll = rng_roll(rne);

	// normal behaviour
	if (roll > 0.03) {
		reward = -std::numeric_limits<real>::infinity();
		for (int i = 0; i < 4; i++) {
			int newx = x, newy = y;
			switch (i) {
				case UP:
					newy++;
					break;
				case DOWN:
					newy--;
					break;
				case RIGHT:
					newx++;
					break;
				case LEFT:
					newx--;
					break;
			}
			if (!(newx < 0 || map.GetWidth() <= newx ||
				newy < 0 || map.GetHeight() <= newy))
			{
				real value = Q[newy*map.GetWidth() + newx][i];
				if (reward < value) {
					reward = value;
					action = (eAction)i;
				}
			}
		}
		if (reward == -std::numeric_limits<real>::infinity()) {
			__debugbreak();
		}
		assert(reward != -std::numeric_limits<real>::infinity());
	}
	// random exploration
	else {
		action = (eAction)rng_action(rne);
		int newx = x, newy = y;
		switch (action) {
			case UP:
				newy++;
				break;
			case DOWN:
				newy--;
				break;
			case RIGHT:
				newx++;
				break;
			case LEFT:
				newx--;
				break;
		}
		if (!(newx < 0 || map.GetWidth() <= newx ||
			newy < 0 || map.GetHeight() <= newy))
		{
			reward = Q[newy*map.GetWidth() + newx][action];
		}
		else {
			reward = -0.04;
		}
		
	}
}

void Agent::Step() {
	// constants
	auto& map = *currentGame->GetMap();

	// get current state
	int x = currentGame->GetCurrentX();
	int y = currentGame->GetCurrentY();
	real r = currentGame->GetCurrentReward();
	size_t indexS = y*map.GetWidth() + x;
	assert(0 <= indexS && indexS < Q.size());

	// update Q
	N[indexS][lastAction]++;
	real Qcurrent = Q[indexS][lastAction];
	real maxNextValue;
	eAction maxNextAction = UP;
	SelectNextStep(x, y, map, maxNextValue, maxNextAction);
	Q[indexS][lastAction] = (1 - alpha)*Qcurrent + alpha*(r + gamma*maxNextValue);

	// perform next step
	currentGame->PerformAction(maxNextAction);
	totalReward += currentGame->GetCurrentReward();

	// update internals
	lastAction = maxNextAction;
}

void Agent::Reset() {
	for (auto& q : Q) {
		for (auto& v : q) {
			v = 0;
		}
	}
	for (auto& n : N) {
		for (auto& v : n) {
			v = 0;
		}
	}
}

void Agent::StartEpisode() {
	totalReward = 0.0f;

	// update Q
	// there's nothing to update, since state == null

	// perform next (first) step
	eAction action;
	real reward;
	SelectNextStep(
		currentGame->GetCurrentX(),
		currentGame->GetCurrentY(),
		*currentGame->GetMap(),
		reward,
		action);

	currentGame->PerformAction(action);
	totalReward += currentGame->GetCurrentReward();

	lastAction = action;
}

float Agent::EndEpisode() {
	// constants
	auto& map = *currentGame->GetMap();

	// get current state
	int x = currentGame->GetCurrentX();
	int y = currentGame->GetCurrentY();
	real r = currentGame->GetCurrentReward();
	size_t indexS = y*map.GetWidth() + x;

	// update Q
	real Qcurrent = Q[indexS][lastAction];
	real maxNextValue;
	eAction maxNextAction;
	SelectNextStep(x, y, map, maxNextValue, maxNextAction);
	Q[indexS][lastAction] = (1 - alpha)*Qcurrent + alpha*(r + gamma*maxNextValue);

	return totalReward;
}


void Agent::SetGame(Game* game) {
	currentGame = game;
	if (game->GetMap()) {
		int w = game->GetMap()->GetWidth();
		int h = game->GetMap()->GetHeight();
		Q.resize(w*h);
		N.resize(w*h);
	}
	Reset();
}


float Agent::GetQ(int x, int y, eAction action) const {
	auto* map = currentGame->GetMap();
	if (!map) {
		return 0;
	}
	if (x < 0 || map->GetWidth() <= x
		|| y < 0 || map->GetHeight() <= y) 
	{
		return 0;
	}
	return Q[y*map->GetWidth() + x][action];
}

float Agent::GetQMax(int x, int y) const {
	auto* map = currentGame->GetMap();
	if (!map) {
		return 0;
	}
	if (x < 0 || map->GetWidth() <= x
		|| y < 0 || map->GetHeight() <= y)
	{
		return 0;
	}
	return *std::max_element(Q[y*map->GetWidth() + x].begin(), Q[y*map->GetWidth() + x].end());
}
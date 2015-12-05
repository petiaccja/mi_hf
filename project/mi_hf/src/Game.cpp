#include "Map.h"
#include "Game.h"
#include "Util.h"

Game::Game() {
	map = nullptr;
}


void Game::PerformAction(eAction action) {
	std::uniform_real_distribution<float> rng(0, 1);
	float roll = rng(rne);
	if (roll < 0.1) {
		// right
		switch (action)
		{
			case UP:
				action = RIGHT;
				break;
			case DOWN:
				action = LEFT;
				break;
			case LEFT:
				action = UP;
				break;
			case RIGHT:
				action = DOWN;
				break;
		}
	}
	else if (roll < 0.2) {
		// left
		switch (action)
		{
			case UP:
				action = LEFT;
				break;
			case DOWN:
				action = RIGHT;
				break;
			case LEFT:
				action = DOWN;
				break;
			case RIGHT:
				action = UP;
				break;
		}
	}


	int deltax, deltay;
	if (action == UP)
		deltay = 1;
	else if (action == DOWN)
		deltay = -1;
	else 
		deltay = 0;

	if (action == RIGHT)
		deltax = 1;
	else if (action == LEFT)
		deltax = -1;
	else
		deltax = 0;

	int newx = posx + deltax;
	int newy = posy + deltay;

	if (newx < 0 || map->GetWidth() <= newx || newy < 0 || map->GetHeight() <= newy) {
		return;
	}
	if ((*map)(newx, newy).type == Map::Field::WALL) {
		return;
	}
	if ((*map)(newx, newy).type == Map::Field::MINE) {
		ended = true;
	}
	if ((*map)(newx, newy).type == Map::Field::FINISH) {
		ended = true;
	}

	posx = newx;
	posy = newy;
}

float Game::GetCurrentReward() const {
	return (*map)(posx, posy).Reward();
}

int Game::GetCurrentX() const {
	return posx;
}

int Game::GetCurrentY() const {
	return posy;
}

void Game::NewGame() {
	posx = 0;
	posy = 0;
	ended = false;
}

bool Game::Ended() {
	return ended;
}

void Game::SetMap(Map* map) {
	this->map = map;
}
#pragma once

#include <random>
#include "Util.h"


class Map;

////////////////////////////////////////////////////////////////////////////////
/// An interactive environment for the 'mines' problem.
/// The game takes place on a rectangular map, 10x10 field by default. There are
/// mines and walls placed on the map in addition to the start and finish fields.
/// The agent has to go from the start to the finish, without blowing itself up.
///	If the agent steps on a mine, it's blown up, and the agent cannot walk through
///	walls.
/// The game implements a reward system, where stepping on each field gives the 
/// agent a certain reward. The reward is a constant value, -0.04 for normal 
/// fields, -1 for mines, undefined for walls and +1 for the finish field.
////////////////////////////////////////////////////////////////////////////////
class Game {
public:
	Game();
	~Game() = default;

	/// The agent performs an action.
	/// \return True, if the game is over (finish or mine), false otherwise.
	bool PerformAction(eAction action);
	/// Get the reward of the current field.
	float GetCurrentReward() const;
	/// Get the current field's x coordinate.
	int GetCurrentX() const;
	/// Get the current field's y coordinate.
	int GetCurrentY() const;

	/// Start a new game.
	/// Puts the agent to the start position.
	void NewGame();
	/// Get wether the game has ended.
	bool Ended();

	/// Set the map of the game.
	/// See Map for more.
	void SetMap(Map* map);
	/// Get the currently set map.
	Map* GetMap() { return map; }
	/// Get the currently set map.
	const Map* GetMap() const { return map; }
private:
	Map* map; ///< Current active map.
	int posx; ///< Agent's current x coordinate.
	int posy; ///< Agent's current y coordinate.
	bool ended; ///< Wether the game has ended.
	std::mt19937 rne;
};
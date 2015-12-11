#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <random>
#include "Util.h"

class Game;
class Map;

////////////////////////////////////////////////////////////////////////////////
/// Realizes an agent that uses Q learning to overcome the 'mines' problem.
///
////////////////////////////////////////////////////////////////////////////////
class Agent {
	using real = double;
public:
	Agent();
	~Agent() = default;

	/// Set a gameing environment in which the agent acts.
	/// \param game The new environment of the agent.
	void SetGame(Game* game);
	/// Reset the agent's learning progress.
	void Reset();

	/// Perform one action in the environment.
	void Step();
	/// Start an episode.
	/// An episode normally starts from the start field and ends when the agent
	/// hits either the finish or a mine. Call everytime a new game has started.
	void StartEpisode();
	/// Ends an episode.
	/// Call everytime the agent's game session is over.
	/// \return The total reward collected during the episode.
	float EndEpisode();

	/// Get an item from the agent's Q(state, action) table.
	/// Use it for debug and display purposes.
	/// \param x The x coordinate of the requested state.
	/// \param y The y coordinate of the requested state.
	/// \param action The action.
	float GetQ(int x, int y, eAction action) const;
	/// Get the best action's value from the Q table.
	/// Returns the max of Q(state, action) for each action.
	/// Use it for debug and display purposes.
	/// \param x The x coordinate of the requested state.
	/// \param y The y coordinate of the requested state.
	float GetQMax(int x, int y) const;

	int GetNSum(int x, int y) const;
private:
	/// Selects the next action of the agent.
	/// Uses a greedy strategy with a little random behaviour.
	/// \param x The current x coordinate of the agent.
	/// \param y The current y coordinate of the agent.
	/// \return The next ideal action to perform.
	eAction SelectNextStep(int x, int y);
	/// Indexing helper for Q table.
	float& Q(int x, int y, eAction action);
	/// Indexing helper for N table.
	int& N(int x, int y, eAction action);

	std::vector<std::array<float, 4>> Q_; ///< Stores the utility of an action at a given state.
	std::vector<std::array<int, 4>> N_; ///< Stores the number an action has been used in a particular state.
	size_t width = 0; ///< Width of the latest set game environment.
	size_t height = 0; ///< Height of the latest set game environment.

	Game* currentGame; ///< Current active game environment.
	real totalReward; ///< The total reward collected during an episode.

	std::mt19937 rne; ///< High quality random number engine.
	std::uniform_real_distribution<real> rng_roll;
	std::uniform_int_distribution<int> rng_action;

	static constexpr real alpha = 0.2f; ///< Learning rate constant.
	static constexpr real gamma = 0.98f; ///< Discount constant.
	static constexpr real explorerness = 0.04f;
};

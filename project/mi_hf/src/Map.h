#pragma once

#include <vector>
#include <random>

////////////////////////////////////////////////////////////////////////////////
/// Map for the 'mines' problem.
/// Contains the fields of the gaming environment. The fields are all one of the
/// predefined ones. See Game for more.
////////////////////////////////////////////////////////////////////////////////
class Map {
public:
	struct Field {
		enum eType {
			FREE,
			MINE,
			WALL,
			FINISH,
		};

		eType type = FREE;

		float Reward() {
			switch (type)
			{
				case Field::FREE:
					return -0.04f;
				case Field::MINE:
					return -1.0f;
				case Field::WALL:
					return -0.04f;
				case Field::FINISH:
					return 1.0f;
				default:
					return 0.0;
			}
		}
	};
public:
	/// Create a map with given size.
	/// \param width Width of the game environment.
	/// \param height Height of the game environment.
	Map(int width, int height);
	~Map();

	/// Resize the game environment.
	/// \param width Width of the game environment.
	/// \param height Height of the game environment.
	void Resize(int width, int height);
	/// Generate a random layout.
	/// \param numWalls The approximate number of walls on the map.
	/// \param numMines The approximate number of mines on the map.
	void Generate(int numWalls, int numMines);

	/// Get field at coordinates.
	Field& operator()(int x, int y);
	/// Get field at coordinates.
	const Field& operator()(int x, int y) const;

	/// Get the width of the map.
	int GetWidth() const { return width; }
	/// Get the height of the map.
	int GetHeight() const { return height; }
private:
	std::vector<Field> fields;
	int width, height;
	std::mt19937 rne;
};
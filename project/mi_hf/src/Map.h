#pragma once

#include <vector>
#include <random>

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
	Map(int width, int height);
	~Map();

	void Resize(int width, int height);
	void Generate(int numWalls, int numMines);

	Field& operator()(int x, int y);
	const Field& operator()(int x, int y) const;

	int GetWidth() const { return width; }
	int GetHeight() const { return height; }
private:
	std::vector<Field> fields;
	int width, height;
	std::mt19937 rne;
};
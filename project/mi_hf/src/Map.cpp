#include "Util.h"
#include "Map.h"

#include <cassert>
#include <ctime>


Map::Map(int width, int height) : rne(Seed()) {
	Resize(width, height);
}
Map::~Map() {

}

void Map::Resize(int width, int height) {
	assert(width > 0 && height > 0);
	fields.resize(width * height);
	this->width = width;
	this->height = height;
}

void Map::Generate(int numWalls, int numMines) {
	std::uniform_int_distribution<int> rngx(0, width-1);
	std::uniform_int_distribution<int> rngy(0, height-1);

	for (auto& field : fields) {
		field.type = Field::FREE;
	}

	int freeFields = width*height;



	while (numWalls > 0 && freeFields > 0) {
		int x = rngx(rne);
		int y = rngy(rne);
		auto& field = (*this)(x, y);
		if (field.type == Field::FREE) {
			field.type = Field::WALL;
			numWalls--;
			freeFields--;
		}
	}

	while (numMines > 0 && freeFields > 0) {
		int x = rngx(rne);
		int y = rngy(rne);
		auto& field = (*this)(x, y);
		if (field.type == Field::FREE) {
			field.type = Field::MINE;
			numMines--;
			freeFields--;
		}
	}
}

auto Map::operator()(int x, int y) -> Field& {
	assert(x < width);
	assert(y < height);
	return fields[y*width + x];
}

auto Map::operator()(int x, int y) const -> const Field& {
	assert(x < width);
	assert(y < height);
	return fields[y*width + x];
}
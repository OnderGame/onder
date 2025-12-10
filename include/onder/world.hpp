#pragma once

#include <cstdint>
#include <onder/collections/array.hpp>
#include <onder/collections/list.hpp>

namespace onder {
namespace world {

struct TileId {
	uint32_t id;

	TileId() : id(0) {}
};

class Chunk {
	collections::Array<collections::Array<TileId, 16>, 16> tiles;
};

class Layer {
};

class World {
	collections::List<Layer> layers;
};

}
}

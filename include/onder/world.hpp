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

// 2**5 * 2**5 * 4 = 2**10 * 4 = 4096 = one page
//
// This may end up useful if/when we start using sparse mmap()ed regions.
const uint8_t CHUNK_DIM_P2 = 5;
const uint8_t CHUNK_DIM = 1 << CHUNK_DIM_P2;
const uint8_t CHUNK_TRIE_DEPTH = 4;
const uint32_t LAYER_DIM = ([]() {
	uint32_t x = 1;
	for (uint8_t i = 0; i < CHUNK_TRIE_DEPTH; i++)
		x *= CHUNK_DIM;
	return x;
})();

template<typename T>
using ChunkGrid = collections::Array<collections::Array<T, CHUNK_DIM>, CHUNK_DIM>;

struct Chunk {
	 ChunkGrid<TileId> tiles; // 2**5
};

struct ChunkRef {
	// 31:10 : offset
	// 8:0   : allocated count (0 = empty)
	//
	// if -1, invalid
	uint32_t offset_count;
	
	ChunkRef() : offset_count(-1) {}
	ChunkRef(uint32_t offset) : offset_count(offset) {}

	bool is_valid() const {
		return offset_count != -1;
	}

	uint32_t offset() {
		return offset_count >> 12;
	}

	uint32_t count() {
		return offset_count & ((1U << 12) - 1);
	}
};

struct ChunkParent {
	ChunkGrid<ChunkRef> tiles;
};

union ChunkSlot {
	Chunk leaf;
	ChunkParent parent;
	ChunkRef ref;
};

struct ChunkRoot {
	ChunkRef ref;
};

class ChunkCollection {
	ChunkSlot *chunks;
	uint32_t max_chunks, head;
	ChunkRef next_free;

	ChunkCollection(const ChunkCollection &) = delete;
	ChunkCollection &operator=(const ChunkCollection &) = delete;

public:
	ChunkCollection();
	~ChunkCollection();

	ChunkRef alloc();

	void free(ChunkRef ref);

	ChunkSlot &get_or_alloc(ChunkRef &index);

	ChunkSlot &operator[](ChunkRef index);
};

class World {
	collections::List<ChunkRoot> layers;
	ChunkCollection chunks;

public:
	World(size_t depth);

	TileId &operator[](uint8_t depth, uint32_t x, uint32_t y);
};

}
}

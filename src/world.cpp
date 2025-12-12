#include <onder/world.hpp>
#include <cstring>
#include <sys/mman.h>

namespace onder {
namespace world {

ChunkCollection::ChunkCollection()
	: chunks(nullptr)
	, max_chunks(0)
	, head(0)
{
	size_t size = 1ULL << 32;
	void *base = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (base == MAP_FAILED)
		throw std::exception();
	chunks = (ChunkSlot *)base;
	max_chunks = size / sizeof(ChunkSlot);
}

ChunkCollection::~ChunkCollection() {
	if (max_chunks > 0)
		munmap(chunks, (size_t)max_chunks * sizeof(ChunkSlot));
}

ChunkRef ChunkCollection::alloc() {
	if (head >= max_chunks)
		throw std::exception();
	// TODO check next_free
	ChunkRef ref(head);
	head += sizeof(*chunks);
	::memset((void *)&chunks[ref.offset()], -1, sizeof(*chunks));
	return ref;
}

ChunkSlot &ChunkCollection::get_or_alloc(ChunkRef &index) {
	if (!index.is_valid())
		index = alloc();
	return chunks[index.offset()];
}

ChunkSlot &ChunkCollection::operator[](ChunkRef index) {
	if (index.offset() << 12 >= head)
		throw std::exception();
	return chunks[index.offset()];
}

World::World(size_t depth, uint8_t layer_size_p2)
	: layers(depth, {})
	, layer_wrap_mask((1ULL << layer_size_p2) - 1)
{
	// TODO dynamically adjust trie depth?
	if (layer_size_p2 < 0 || layer_size_p2 > 20)
		throw std::exception();
}

Chunk &World::chunk(uint8_t depth, uint32_t cx, uint32_t cy) {
	cx &= layer_wrap_mask / CHUNK_DIM, cy &= layer_wrap_mask / CHUNK_DIM;
	auto f = [](auto v, auto i) { return (v >> (i * CHUNK_DIM_P2)) % CHUNK_DIM; };
	ChunkRef *ref = &layers[depth].ref;
	for (uint8_t i = CHUNK_TRIE_DEPTH; i > 0;) {
		i--;
		ChunkSlot &cur = chunks.get_or_alloc(*ref);
		auto px = f(cx, i), py = f(cy, i);
		ref = &cur.parent.tiles[py][px];
	}
	return chunks.get_or_alloc(*ref).leaf;
}

TileId &World::operator[](uint8_t depth, uint32_t x, uint32_t y) {
	Chunk &c = chunk(depth, x / CHUNK_DIM, y / CHUNK_DIM);
	return c.tiles[y % CHUNK_DIM][x % CHUNK_DIM];
}

}
}

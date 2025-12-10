#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <onder/collections/slice.hpp>

namespace onder {
namespace filesystem {

class FileMmap {
	const uint8_t *base;
	size_t len;

public:
	FileMmap() : base(nullptr), len(0) {}
	FileMmap(const char *path) : FileMmap() {
		open(path);
	}
	~FileMmap() {
		close();
	}

	void open(const char *path);

	void close() {
		if (len != 0)
			munmap((void *)base, len);
	}

	collections::Slice<const uint8_t> slice() const {
		return { base, len };
	}
};

}
}

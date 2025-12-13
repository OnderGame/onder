#include <onder/filesystem.hpp>

namespace onder {
namespace filesystem {

void FileMmap::open(const char *path) {
	close();
	int fd = ::open(path, O_RDONLY);
	struct stat stat;
	if (fd < 0)
		throw std::exception();
	if (fstat(fd, &stat) < 0) {
		::close(fd);
		throw std::exception();
	}
	void *res = mmap(nullptr, (size_t)stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (res == MAP_FAILED) {
		::close(fd);
		throw std::exception();
	}
	::close(fd);
	base = (const uint8_t *)res;
	len = (size_t)stat.st_size;
}

}
}

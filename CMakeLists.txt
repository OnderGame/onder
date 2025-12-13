cmake_minimum_required(VERSION 3.22)
project(Onder)

set(CMAKE_CXX_STANDARD 23)

option(ENABLE_EXAMPLES   "Enable examples." NO)
option(PIXELTOASTER_NO_STL "Disable use of STL library." YES)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/lodepng)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/pixeltoaster)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/pixeltoaster)

add_executable(Onder src/main.cpp src/filesystem.cpp src/graphics.cpp src/net.cpp src/world.cpp src/multiplayer/client.cpp src/multiplayer/server.cpp thirdparty/lodepng/lodepng.cpp)
target_link_libraries(Onder PixelToaster)

# https://stackoverflow.com/a/50882216
if(MSVC)
	target_compile_options(Onder PRIVATE /W4 /WX)
else()
	# https://old.reddit.com/r/cpp/comments/zde4cu
	target_compile_options(Onder PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
	# too noisy + it's a dependency so *shrug*
	set_source_files_properties(thirdparty/lodepng/lodepng.cpp PROPERTIES COMPILE_FLAGS "-Wno-conversion")
endif()

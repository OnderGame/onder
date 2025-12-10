cmake_minimum_required(VERSION 3.22)
project(Onder)

set(CMAKE_CXX_STANDARD 23)

option(ENABLE_EXAMPLES   "Enable examples." NO)
option(PIXELTOASTER_NO_STL "Disable use of STL library." YES)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/pixeltoaster)
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/pixeltoaster)

add_executable(Onder src/main.cpp)
target_link_libraries(Onder PixelToaster)

# CMakeList.txt : CMake project for YoutubeDlCleanUp, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("youtubedl-cleanup")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source to this project's executable.
add_executable (youtubedl-cleanup "youtubedl-cleanup.cpp" )

# TODO: Add tests and install targets if needed.

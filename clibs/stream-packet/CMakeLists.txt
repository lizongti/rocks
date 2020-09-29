cmake_minimum_required(VERSION 2.4)
project(lstream_packet)

set(CMAKE_MACOSX_RPATH 1)
set(LIBRARIES_FROM_REFERENCES "")
set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++17 -fPIC -O3")

add_executable(test test/test.cpp src/core.cpp)
add_library(stream_packet SHARED src/luajit.cpp src/core.cpp)
target_link_libraries(stream_packet -L/usr/local/lib -L/usr/lib -ljemalloc -lluajit-5.1)
include_directories("${PROJECT_SOURCE_DIR}/src" "/usr/local/include" "/usr/local/include/luajit-2.0")
install(FILES build/libstream_packet.so DESTINATION /usr/local/lib/lua/5.1/ RENAME stream_packet.so 
PERMISSIONS WORLD_EXECUTE WORLD_READ OWNER_EXECUTE OWNER_READ OWNER_WRITE GROUP_EXECUTE GROUP_READ)
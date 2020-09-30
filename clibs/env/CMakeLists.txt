cmake_minimum_required(VERSION 2.4)
project(env)

set(CMAKE_MACOSX_RPATH 1)
set(LIBRARIES_FROM_REFERENCES "")
set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fPIC -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++17 -fPIC -O3")

add_library(env SHARED lenv.c)
target_link_libraries(env -L/usr/local/lib -L/usr/lib -ljemalloc -lluajit-5.1)
include_directories("${PROJECT_SOURCE_DIR}/src" "/usr/include/" "/usr/local/include/luajit-2.0/")
install(FILES build/libenv.so DESTINATION /usr/local/lib/lua/5.1/ RENAME env.so PERMISSIONS WORLD_EXECUTE WORLD_READ OWNER_EXECUTE OWNER_READ OWNER_WRITE GROUP_EXECUTE GROUP_READ)
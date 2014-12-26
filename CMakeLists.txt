cmake_minimum_required( VERSION 2.6 )

project( Intro )

set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules )

set(CMAKE_C_FLAGS "-std=c99")

# the name of the target operating system
#SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
#SET(CMAKE_C_COMPILER i686-w64-mingw32-cc)

# here is the target environment located
#SET(CMAKE_FIND_ROOT_PATH  /usr/i686-w64-mingw32 )

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

add_subdirectory(core)
add_subdirectory(analyse)
add_subdirectory(extract)
add_subdirectory(analyse-gtk)


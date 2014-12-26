find_package(ZLIB)

add_library (analyse analyse.c analyse.h output.c output.h )

target_link_libraries (analyse core ${ZLIB_LIBRARIES})
target_include_directories (analyse PUBLIC ${ZLIB_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})


#add_executable (analyse-main main_analyse.c)
#target_link_libraries (analyse-main LINK_PUBLIC analyse)

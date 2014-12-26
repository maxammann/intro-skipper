add_library (extract extract.c extract.h)
target_include_directories (extract PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (extract LINK_PUBLIC core)

add_executable (extract-main main_extract.c)
target_link_libraries (extract-main LINK_PUBLIC analyse)

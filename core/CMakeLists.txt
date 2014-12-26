find_package( AVFormat )
find_package( AVCodec )
find_package( AVUtil )
find_package( AVFilter )
find_package( SWScale )
find_package( Jansson )
find_package(Threads)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

set(	
	INCLUDES
	${AVFormat_INCLUDE_DIRS}
	${AVCodec_INCLUDE_DIRS}
	${AVUtil_INCLUDE_DIRS}
	${AVFilter_INCLUDE_DIRS}
	${LIBSWSCALE_INCLUDE_DIRS}
	${JANSSON_INCLUDE_DIRS}
	${GTK3_INCLUDE_DIRS}
)

set( 
	LIBRARIES
	${AVFormat_LIBRARIES}
	${AVCodec_LIBRARIES}
	${AVUtil_LIBRARIES}
	${AVFilter_LIBRARIES}
	${LIBSWSCALE_LIBRARIES}
	${JANSSON_LIBRARIES}
	${GTK3_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include_directories(${INCLUDES})
add_definitions(${GTK3_CFLAGS_OTHER})

add_library (core frame_io.h frame_util.h json_util.h skimmer.h frame_io.c frame_util.c json_util.c skimmer.c)
target_include_directories (core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (core PUBLIC ${INCLUDES})

target_link_libraries(
	core
	${LIBRARIES}
)

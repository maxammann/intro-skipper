include(GResource)
gresource(${CMAKE_CURRENT_SOURCE_DIR} gresource.xml
	${CMAKE_CURRENT_BINARY_DIR} RESOURCE_FILE)

add_executable (analyse_gtk main_analyse_gtk.c gdk_util.c ${RESOURCE_FILE})
target_link_libraries (analyse_gtk LINK_PUBLIC analyse extract )

#add_executable (analyse_gtk_test test.c gdk_util.c test_app.c test_window.c)
#target_link_libraries (analyse_gtk_test LINK_PUBLIC analyse extract )

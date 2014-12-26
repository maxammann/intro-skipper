include(LibAVFindComponent)

libav_find_component("Device")

# Handle arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AVDevice
	DEFAULT_MSG
	AVDevice_LIBRARIES
	AVDevice_INCLUDE_DIRS
)

mark_as_advanced(AVDevice_LIBRARIES AVDevice_INCLUDE_DIRS)

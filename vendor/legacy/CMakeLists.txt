add_subdirectory(cryptopp)
add_subdirectory(lzo-2.10)
add_subdirectory(mio)

## zstd is a bit tricky
set(ZSTD_BUILD_SHARED OFF CACHE BOOL "BUILD SHARED LIBRARIES" FORCE)
add_subdirectory(zstd-1.5.7/build/cmake zstd)
include_directories("zstd/lib")

if (WIN32)
	set_target_properties(cryptopp-static PROPERTIES FOLDER vendor)
	set_target_properties(lzo2 PROPERTIES FOLDER vendor)
	
	## zstd stuff
	set_target_properties(zstd PROPERTIES FOLDER vendor/zstd)
	set_target_properties(libzstd_static PROPERTIES FOLDER vendor/zstd)
	set_target_properties(clean-all PROPERTIES FOLDER vendor/zstd)
	set_target_properties(uninstall PROPERTIES FOLDER vendor/zstd)
endif()
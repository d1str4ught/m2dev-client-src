add_subdirectory(lzo-2.10)
add_subdirectory(mio)
add_subdirectory(libsodium)

## zstd is a bit tricky
set(ZSTD_BUILD_SHARED OFF CACHE BOOL "BUILD SHARED LIBRARIES" FORCE)
add_subdirectory(zstd-1.5.7/build/cmake zstd)
include_directories("zstd/lib")

## FreeType - disable optional dependencies we don't need
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
add_subdirectory(freetype-2.13.3)

if (WIN32)
	set_target_properties(lzo2 PROPERTIES FOLDER vendor)
	set_target_properties(sodium PROPERTIES FOLDER vendor)
	set_target_properties(freetype PROPERTIES FOLDER vendor)

	## zstd stuff
	set_target_properties(zstd PROPERTIES FOLDER vendor/zstd)
	set_target_properties(libzstd_static PROPERTIES FOLDER vendor/zstd)
	set_target_properties(clean-all PROPERTIES FOLDER vendor/zstd)
	set_target_properties(uninstall PROPERTIES FOLDER vendor/zstd)
endif()
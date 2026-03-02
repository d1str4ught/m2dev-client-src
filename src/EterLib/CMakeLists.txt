file(GLOB_RECURSE FILE_SOURCES "*.h" "*.c" "*.cpp")

add_library(EterLib STATIC ${FILE_SOURCES})

target_link_libraries(EterLib
	sodium
	mio
	freetype
	d3d11
	dxgi
	d3dcompiler
)

GroupSourcesByFolder(EterLib)

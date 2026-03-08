add_library(Python INTERFACE)
target_include_directories(Python INTERFACE "${CMAKE_SOURCE_DIR}/extern/include")
target_link_libraries(Python INTERFACE
	"${CMAKE_CURRENT_LIST_DIR}/python314.lib"
	pathcch
	bcrypt
)

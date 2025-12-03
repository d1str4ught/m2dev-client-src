cmake_minimum_required(VERSION 3.19)

project(m2dev-client-src)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ASan support
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

set(CMAKE_MODULE_PATH
	${CMAKE_MODULE_PATH}
	"${CMAKE_CURRENT_SOURCE_DIR}/buildtool"
)

include(Utils)

set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Release" CACHE STRING "Configurations" FORCE)
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo,Release>:ProgramDatabase>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG")

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options(/MP)

add_compile_options(
	$<$<CXX_COMPILER_ID:MSVC>:/wd4828>
	$<$<CXX_COMPILER_ID:MSVC>:/wd4996>
)

# ASan flags
if(ENABLE_ASAN)
	if(MSVC)
		add_compile_options(/fsanitize=address)
		add_link_options(/fsanitize=address)
		add_definitions(-D_DISABLE_VECTOR_ANNOTATION)
		add_definitions(-D_DISABLE_STRING_ANNOTATION)
	else()
		add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
		add_link_options(-fsanitize=address)
	endif()
endif()

add_definitions(-DNOMINMAX)
add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DWINDOWS_IGNORE_PACKING_MISMATCH)

add_definitions(-DUSE_LOD)
add_definitions(-DDUNGEON_WORK)
add_definitions(-DBUILDING_GRANNY_STATIC)
add_definitions(-DGRANNY_THREADED)
add_definitions(-DPy_NO_ENABLE_SHARED)

add_compile_definitions("$<$<CONFIG:Debug>:DEBUG>")
add_compile_definitions("$<$<CONFIG:RelWithDebInfo>:_DISTRIBUTE>")
add_compile_definitions("$<$<CONFIG:Release>:_DISTRIBUTE>")
# ====================================================================
# [START] Centralized Submodule Initialization (Client)
# Ensure dependencies are available before compilation or linking starts.
# ====================================================================

# Ensure this is a Git repository before proceeding
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
	# 1. Locate the Python interpreter (Required to run depinit_client.py)
	find_package(Python3 REQUIRED COMPONENTS Interpreter)

	# 2. Execute the dependency initialization script
	message(STATUS "Running client dependency initialization script (depinit.py)...")

	execute_process(
		COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/depinit.py"
		# Ensure execution happens in the project root for Git commands
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" 
		RESULT_VARIABLE DEP_INIT_RESULT
		OUTPUT_VARIABLE DEP_INIT_OUTPUT
		ERROR_VARIABLE DEP_INIT_OUTPUT
	)

	# 3. Print the script output
	message(STATUS "${DEP_INIT_OUTPUT}")

	# 4. Check the return code and halt if the script failed
	if (DEP_INIT_RESULT)
		message(FATAL_ERROR "Client dependency initialization script failed! See messages above.")
	endif()

	message(STATUS "All client dependencies are ready.")
endif()

# ====================================================================
# [END] Centralized Submodule Initialization (Client)
# ====================================================================

include_directories("src")
include_directories("extern/include")

# Add subdirectories for libraries and executables
add_subdirectory(vendor)
add_subdirectory(src)
add_subdirectory(extern)
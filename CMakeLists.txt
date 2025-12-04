cmake_minimum_required(VERSION 3.19)

project(m2dev-client-src)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ASan support
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

# Custom options
option(USE_LEGDEPS "Use preincluded legacy dependencies (vendor/legacy instead of git submodules installation via depinit)" ON)

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
if (NOT USE_LEGDEPS)
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
endif()

# ====================================================================
# [END] Centralized Submodule Initialization (Client)
# ====================================================================

include_directories("src")
include_directories("extern/include/common")

if (USE_LEGDEPS)
	include_directories("extern/include/legacy")
else()
	include_directories("extern/include/latest")
endif()

if (USE_LEGDEPS)
	# For MSVC we must ensure Crypto++ is built as static-only and avoid its asm duplicates.
	# This forces the vendor/cryptopp subproject to:
	#  - disable asm (avoids x64dll.asm duplicate symbols on MSVC toolchains),
	#  - build only the static library (prevents building a separate shared cryptopp lib).
	if (MSVC)
	  # Disable Crypto++ assembly on MSVC (avoids duplicate symbol issues)
	  set(DISABLE_ASM ON CACHE BOOL "Disable Crypto++ ASM on MSVC to avoid duplicate symbols" FORCE)

	  # Force Crypto++ to build static-only (no cryptopp shared target)
	  set(BUILD_SHARED OFF CACHE BOOL "Force Crypto++ shared build OFF on MSVC" FORCE)
	  set(BUILD_STATIC ON CACHE BOOL "Force Crypto++ static build ON on MSVC" FORCE)
	endif()
endif()

# Add subdirectories for libraries and executables
if (USE_LEGDEPS)
	add_subdirectory(vendor/legacy)
else()
	add_subdirectory(vendor/latest)
endif()

add_subdirectory(src)
add_subdirectory(extern)
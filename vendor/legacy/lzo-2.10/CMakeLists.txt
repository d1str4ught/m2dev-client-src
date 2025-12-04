cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(lzo2 VERSION 2.10 LANGUAGES C)

# Collect sources
file(GLOB lzo2_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)
list(SORT lzo2_SOURCES)

# Static library
add_library(lzo2 STATIC ${lzo2_SOURCES})

# Public headers for consumers (works when added as a subdir)
target_include_directories(lzo2
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Nice-to-haves
set_target_properties(lzo2 PROPERTIES
    OUTPUT_NAME "lzo2"                 # lib name on disk
    POSITION_INDEPENDENT_CODE ON       # useful if linked into shared libs
)

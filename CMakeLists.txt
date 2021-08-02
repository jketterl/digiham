cmake_minimum_required (VERSION 3.0)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

project (digiham VERSION 0.5.0)
add_definitions(-DVERSION="${PROJECT_VERSION}-dev")

enable_language(CXX)
set(CMAKE_CXX_STANDARD 11)

include(GNUInstallDirs)

find_package(CodecServer REQUIRED)

find_package(Csdr REQUIRED)

SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_C_FLAGS_RELEASE "-O3")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -O3")
SET(CMAKE_C_FLAGS_DEBUG "-g -O3")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/lib)

add_subdirectory(src)

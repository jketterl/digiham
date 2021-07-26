add_library(digiham STATIC dumphex.c hamming_distance.c version.c decoder.cpp meta.cpp phase.cpp)
file(GLOB DIGIHAM_HEADERS "*.h" "*.hpp")
set_target_properties(digiham PROPERTIES PUBLIC_HEADER "${DIGIHAM_HEADERS}")
target_include_directories(digiham INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
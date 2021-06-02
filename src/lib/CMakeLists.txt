add_library(digiham STATIC dumphex.c hamming_distance.c version.c)
file(GLOB DIGIHAM_HEADERS "*.h")
set_target_properties(digiham PROPERTIES PUBLIC_HEADER "${DIGIHAM_HEADERS}")
target_include_directories(digiham INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
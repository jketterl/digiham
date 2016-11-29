cmake_minimum_required (VERSION 2.6)
project (digiham)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

find_package(LibMbe REQUIRED)

INCLUDE_DIRECTORIES(
    "${LIBMBE_INCLUDE_DIR}"
)

SET(LIBS ${LIBS} ${LIBMBE_LIBRARY})

add_library(ysf_trellis ysf_trellis.c)
target_include_directories (ysf_trellis PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_library(ysf_golay ysf_golay.c)
target_include_directories (ysf_golay PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_library(ysf_fich ysf_fich.c)
target_include_directories (ysf_fich PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_library(ysf_whitening ysf_whitening.c)
target_include_directories (ysf_whitening PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(rrc_filter rrc_filter.c)
add_executable(gfsk_demodulator gfsk_demodulator.c)
add_executable(dmr_decoder dmr_decoder.c)
add_executable(ysf_decoder ysf_decoder.c)
target_link_libraries(ysf_decoder ysf_trellis ysf_golay ysf_fich ysf_whitening)
add_executable(mbe_synthesizer mbe_synthesizer.c)
TARGET_LINK_LIBRARIES(mbe_synthesizer ${LIBS})

install(TARGETS rrc_filter DESTINATION bin)
install(TARGETS gfsk_demodulator DESTINATION bin)
install(TARGETS dmr_decoder DESTINATION bin)
install(TARGETS ysf_decoder DESTINATION bin)
install(TARGETS mbe_synthesizer DESTINATION bin)

SET(CMAKE_CXX_FLAGS "-O3")
SET(CMAKE_C_FLAGS "-O3")

cmake_minimum_required (VERSION 2.6)
project (digiham)
add_executable(rrc_filter rrc_filter.c)
add_executable(gfsk_demodulator gfsk_demodulator.c)

install(TARGETS rrc_filter DESTINATION bin)
install(TARGETS gfsk_demodulator DESTINATION bin)

SET(CMAKE_CXX_FLAGS "-O3")
SET(CMAKE_C_FLAGS "-O3")

cmake_minimum_required (VERSION 2.6)
project (digiham)
add_executable(rrc_filter rrc_filter.c)

install(TARGETS rrc_filter DESTINATION bin)

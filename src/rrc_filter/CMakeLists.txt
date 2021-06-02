add_executable(rrc_filter rrc_filter.c)
target_link_libraries(rrc_filter digiham)
install(TARGETS rrc_filter DESTINATION ${CMAKE_INSTALL_BINDIR})
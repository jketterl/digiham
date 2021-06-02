add_executable(pocsag_decoder pocsag_decoder.c bch_31_21.c)
target_link_libraries(pocsag_decoder digiham)
install(TARGETS pocsag_decoder DESTINATION ${CMAKE_INSTALL_BINDIR})
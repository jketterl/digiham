add_executable(fsk_demodulator fsk_demodulator.c)
target_link_libraries(fsk_demodulator digiham)
install(TARGETS fsk_demodulator DESTINATION ${CMAKE_INSTALL_BINDIR})
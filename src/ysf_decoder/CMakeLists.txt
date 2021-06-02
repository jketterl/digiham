add_executable(ysf_decoder ysf_decoder.c crc16.c fich.c golay_24_12.c gps.c radio_types.c trellis.c whitening.c)
target_link_libraries(ysf_decoder digiham)
install(TARGETS ysf_decoder DESTINATION ${CMAKE_INSTALL_BINDIR})
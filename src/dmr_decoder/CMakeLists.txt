add_executable(dmr_decoder dmr_decoder.c bptc_196_96.c golay_20_8.c hamming_7_4.c hamming_13_9.c hamming_15_11.c hamming_16_11.c quadratic_residue.c)
target_link_libraries(dmr_decoder digiham)
install(TARGETS dmr_decoder DESTINATION ${CMAKE_INSTALL_BINDIR})
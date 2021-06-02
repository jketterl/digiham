add_executable(ysf_decoder ysf_decoder.c crc16.c fich.c golay_24_12.c gps.c radio_types.c trellis.c whitening.c)
target_link_libraries(ysf_decoder digiham)
install(TARGETS ysf_decoder DESTINATION ${CMAKE_INSTALL_BINDIR})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(ysf_golay_24_12_syndrome_generator golay_24_12 golay_24_12_syndrome_generator.c)
endif()
add_library(digiham_ysf_decoder OBJECT ysf_decoder.cpp ysf_phase.cpp ysf_meta.cpp fich.cpp trellis.c golay_24_12.c crc16.c whitening.c data.cpp gps.c radio_types.c)
target_compile_options(digiham_ysf_decoder PRIVATE "-fPIC")

add_executable(ysf_decoder ysf_cli.cpp)
target_link_libraries(ysf_decoder digiham)
install(TARGETS ysf_decoder DESTINATION ${CMAKE_INSTALL_BINDIR})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(ysf_golay_24_12_syndrome_generator golay_24_12 golay_24_12_syndrome_generator.c)
endif()
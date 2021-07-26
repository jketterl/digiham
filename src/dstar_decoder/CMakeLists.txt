#add_library(digiham_dstar OBJECT )

add_executable(dstar_decoder dstar_cli.cpp dstar_decoder.cpp header.cpp dstar_phase.cpp scrambler.cpp crc.cpp dstar_meta.cpp)
target_link_libraries(dstar_decoder digiham csdr++)
install(TARGETS dstar_decoder DESTINATION ${CMAKE_INSTALL_BINDIR})
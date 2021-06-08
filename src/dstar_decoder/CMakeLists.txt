add_executable(dstar_decoder dstar_decoder.cpp header.cpp phase.cpp scrambler.cpp crc.cpp meta.cpp)
target_link_libraries(dstar_decoder digiham)
install(TARGETS dstar_decoder DESTINATION ${CMAKE_INSTALL_BINDIR})
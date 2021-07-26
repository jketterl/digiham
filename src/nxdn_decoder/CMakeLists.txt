add_executable(nxdn_decoder nxdn_decoder.cpp nxdn_meta.cpp nxdn_phase.cpp lich.cpp sacch.cpp scrambler.cpp facch1.cpp trellis.cpp)
target_link_libraries(nxdn_decoder digiham csdr++)
install(TARGETS nxdn_decoder DESTINATION ${CMAKE_INSTALL_BINDIR})
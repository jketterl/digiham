add_library(digiham_nxdn OBJECT  nxdn_decoder.cpp nxdn_meta.cpp nxdn_phase.cpp lich.cpp sacch.cpp scrambler.cpp facch1.cpp trellis.cpp)
target_compile_options(digiham_nxdn PRIVATE "-fPIC")

add_executable(nxdn_decoder nxdn_cli.cpp)
target_link_libraries(nxdn_decoder digiham)
install(TARGETS nxdn_decoder DESTINATION ${CMAKE_INSTALL_BINDIR})
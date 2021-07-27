add_library(digiham_dstar OBJECT  dstar_decoder.cpp header.cpp dstar_phase.cpp scrambler.cpp crc.cpp dstar_meta.cpp)
target_compile_options(digiham_dstar PRIVATE "-fPIC")

add_executable(dstar_decoder dstar_cli.cpp)
target_link_libraries(dstar_decoder digiham)
install(TARGETS dstar_decoder DESTINATION ${CMAKE_INSTALL_BINDIR})
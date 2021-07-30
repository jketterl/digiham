add_library(digiham_gfsk_demodulator OBJECT gfsk_demodulator.cpp)
target_compile_options(digiham_gfsk_demodulator PRIVATE "-fPIC")

add_executable(gfsk_demodulator gfsk_demodulator_cli.cpp)
target_link_libraries(gfsk_demodulator digiham m)
install(TARGETS gfsk_demodulator DESTINATION ${CMAKE_INSTALL_BINDIR})
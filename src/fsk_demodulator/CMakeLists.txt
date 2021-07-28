add_library(digiham_fsk_demodulator OBJECT fsk_demodulator.cpp)
target_compile_options(digiham_fsk_demodulator PRIVATE "-fPIC")

add_executable(fsk_demodulator fsk_demodulator_cli.cpp)
target_link_libraries(fsk_demodulator digiham m)
install(TARGETS fsk_demodulator DESTINATION ${CMAKE_INSTALL_BINDIR})
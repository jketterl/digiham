add_executable(nxdn_decoder nxdn_decoder.cpp nxdn_meta.cpp nxdn_phase.cpp)
target_link_libraries(nxdn_decoder digiham)
install(TARGETS nxdn_decoder DESTINATION ${CMAKE_INSTALL_BINDIR})
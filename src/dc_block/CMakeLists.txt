add_executable(dc_block dc_block.c)
target_link_libraries(dc_block digiham)
install(TARGETS dc_block DESTINATION ${CMAKE_INSTALL_BINDIR})
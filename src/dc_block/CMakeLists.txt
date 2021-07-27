add_library(digiham_dc_block OBJECT dc_block.cpp)
target_compile_options(digiham_dc_block PRIVATE "-fPIC")

add_executable(dc_block dc_block_cli.cpp)
target_link_libraries(dc_block digiham)
install(TARGETS dc_block DESTINATION ${CMAKE_INSTALL_BINDIR})
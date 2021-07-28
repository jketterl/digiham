add_library(digiham_rrc_filter OBJECT rrc_filter.cpp)
target_compile_options(digiham_rrc_filter PRIVATE "-fPIC")

add_executable(rrc_filter rrc_filter_cli.cpp)
target_link_libraries(rrc_filter digiham)
install(TARGETS rrc_filter DESTINATION ${CMAKE_INSTALL_BINDIR})
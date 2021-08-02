add_executable(mbe_synthesizer mbe_synthesizer.cpp cli.cpp)
target_link_libraries(mbe_synthesizer CodecServer::codecserver)
install(TARGETS mbe_synthesizer DESTINATION ${CMAKE_INSTALL_BINDIR})
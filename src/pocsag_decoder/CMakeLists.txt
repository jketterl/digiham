add_executable(pocsag_decoder pocsag_decoder.c bch_31_21.c)
target_link_libraries(pocsag_decoder digiham)
install(TARGETS pocsag_decoder DESTINATION ${CMAKE_INSTALL_BINDIR})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(bch_31_21_syndrome_generator bch_31_21.c bch_31_21_syndrome_generator.c)
endif()
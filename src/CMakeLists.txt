install(EXPORT DigihamTargets
    FILE DigihamTargets.cmake
    NAMESPACE Digiham::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Digiham
)

include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/DigihamConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Digiham
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/DigihamConfig.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Digiham
)

add_subdirectory(lib)
add_subdirectory(digitalvoice_filter)
add_subdirectory(dmr_decoder)
add_subdirectory(fsk_demodulator)
add_subdirectory(gfsk_demodulator)
add_subdirectory(mbe_synthesizer)
add_subdirectory(pocsag_decoder)
add_subdirectory(rrc_filter)
add_subdirectory(ysf_decoder)
add_subdirectory(dstar_decoder)
add_subdirectory(nxdn_decoder)

add_library(digiham SHARED
    $<TARGET_OBJECTS:digiham_util>
    $<TARGET_OBJECTS:digiham_dstar>
    $<TARGET_OBJECTS:digiham_nxdn>
    $<TARGET_OBJECTS:digiham_fsk_demodulator>
    $<TARGET_OBJECTS:digiham_digitalvoice_filter>
    $<TARGET_OBJECTS:digiham_rrc_filter>
    $<TARGET_OBJECTS:digiham_mbe>
    $<TARGET_OBJECTS:digiham_gfsk_demodulator>
    $<TARGET_OBJECTS:digiham_dmr_decoder>
    $<TARGET_OBJECTS:digiham_ysf_decoder>
    $<TARGET_OBJECTS:digiham_pocsag>
)
target_link_libraries(digiham Csdr::csdr++ CodecServer::codecserver ICU::uc)
set_target_properties(digiham PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
file(GLOB DIGIHAM_HEADERS "${PROJECT_SOURCE_DIR}/include/*.hpp" "${PROJECT_SOURCE_DIR}/include/*.h")
set_target_properties(digiham PROPERTIES PUBLIC_HEADER "${DIGIHAM_HEADERS}")

install(TARGETS digiham
    EXPORT DigihamTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/digiham
)


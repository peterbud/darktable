include_directories(${CMAKE_CURRENT_BINARY_DIR}/..)
add_executable(darktable-library main.c wnd_main.c)

target_link_libraries(darktable-library ${GTK3_LIBRARIES})
add_definitions(${GTK3_DEFINITIONS})

target_link_libraries(darktable-library lib_darktable)

set_target_properties(darktable-library PROPERTIES LINKER_LANGUAGE C)

if (WIN32)
  set_target_properties(darktable-library PROPERTIES LINK_FLAGS "-mwindows -Wl,-subsystem,windows")
  _detach_debuginfo (darktable-library bin)
endif(WIN32)

install(TARGETS darktable-library DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT DTApplication)
install(FILES librarymgmt.glade DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/darktable COMPONENT DTApplication)

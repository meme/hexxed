set(MAN_PAGES hexxed.1 hexxed-tutorial.7)
set(MAN_OUTPUT)

foreach(PAGE IN LISTS MAN_PAGES)
  set(OUTPUT ${CMAKE_BINARY_DIR}/${PAGE})
  set(SOURCE ${CMAKE_SOURCE_DIR}/man/${PAGE}.scd)

  add_custom_command(OUTPUT ${OUTPUT}
    COMMAND "scdoc" "<" ${SOURCE} ">" ${OUTPUT}
    DEPENDS ${SOURCE}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
  )

  list(APPEND MAN_OUTPUT ${OUTPUT})

  get_filename_component(MAN_EXTENSION ${PAGE} EXT)
  string(SUBSTRING ${MAN_EXTENSION} 1 1 MAN_SECTION)
  install(FILES ${OUTPUT} DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man${MAN_SECTION})
endforeach()

add_custom_target(man ALL DEPENDS ${MAN_OUTPUT})

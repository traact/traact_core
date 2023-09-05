
set(CONSOLE_SOURCES
        traactConsole.cpp
        )

add_executable(traactConsole ${CONSOLE_SOURCES})

target_link_libraries(traactConsole PUBLIC
        traact_core
        )


install(TARGETS traactConsole
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
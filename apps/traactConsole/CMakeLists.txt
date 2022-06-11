
set(CONSOLE_SOURCES
        traactConsole.cpp
        )

add_executable(traactConsole ${CONSOLE_SOURCES})

target_link_libraries(traactConsole PUBLIC
        ${CONAN_PACKAGE_NAME}
        )
target_include_directories(traactConsole PRIVATE ../../src)


install(TARGETS traactConsole
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows

set(TEST_SOURCES
        test_main.cpp
        )

add_executable(testProgram ${TEST_SOURCES})

target_link_libraries(testProgram PUBLIC
        ${CONAN_PACKAGE_NAME}
        )

target_link_libraries(testProgram PUBLIC
        CONAN_PKG::opencv
        CONAN_PKG::TBB
        CONAN_PKG::spdlog
        CONAN_PKG::rttr
        CONAN_PKG::eigen

        )

# conan weirdness ...
if (TARGET CONAN_PKG::GTest)
    target_link_libraries(testProgram PUBLIC CONAN_PKG::GTest)
else ()
    target_link_libraries(testProgram PUBLIC CONAN_PKG::gtest)
endif ()

if (WITH_COVERAGE)
    message("link gcov")
    target_link_libraries(testProgram PUBLIC gcov)
endif ()

target_include_directories(testProgram
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        )


add_executable(RTTRPluginTest simple_apps/RTTPluginTest.cpp)

target_link_libraries(RTTRPluginTest PUBLIC
        ${CONAN_PACKAGE_NAME}
        CONAN_PKG::opencv
        CONAN_PKG::TBB
        CONAN_PKG::spdlog
        CONAN_PKG::rttr
        )

add_executable(CreateObjectest simple_apps/CreateObjectExperiment.cpp)

target_link_libraries(CreateObjectest PUBLIC
        ${CONAN_PACKAGE_NAME}
        CONAN_PKG::opencv
        CONAN_PKG::TBB
        CONAN_PKG::spdlog
        CONAN_PKG::rttr
        )
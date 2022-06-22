cmake_minimum_required(VERSION 3.16)


if (MSVC)
    # per default disable extended aligned storage for now on msvc
    add_definitions(-D_DISABLE_EXTENDED_ALIGNED_STORAGE -DHAVE_SNPRINTF)
endif ()

# public header
set(TEST_SOURCES
        main.cpp
        #dataflow/tbb/Test_DynamicJoinNode.cpp
        #dataflow/tbb/DataflowMessageReceiver.cpp
        #dataflow/tbb/DataflowMessageReceiver.h
        buffer/SingleComponentBufferTest.cpp
        TestDatatypes.h TestDatatypes.cpp TestComponents.cpp TestComponents.h TestAsyncSourceComponent.cpp TestAsyncSourceComponent.h TestSyncSinkComponent.cpp TestSyncSinkComponent.h dataflow/Test_00_SourceSink.cpp TestComponentState.cpp TestComponentState.h dataflow/Test_01_TwoAsyncSource_SyncFunctional_SyncSink.cpp TestSyncFunctionalComponent.cpp TestSyncFunctionalComponent.h dataflow/taskflow/Test_TimeDomainClock.cpp pattern/test_serialization.cpp)

add_executable(core_tests ${TEST_SOURCES})

target_link_libraries(core_tests PUBLIC
        ${CONAN_PACKAGE_NAME}
        )

target_include_directories(core_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_BINARY_DIR}
        )

install(TARGETS core_tests
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows

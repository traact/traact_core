
find_package(GTest)
find_package(ApprovalTests)

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
        traact_core GTest::gtest ApprovalTests::ApprovalTests 
        )

target_include_directories(core_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src        
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

install(TARGETS core_tests
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_COMPONENTTYPES_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_COMPONENTTYPES_H_
#include <traact/traact_core_export.h>
namespace traact::component {
enum class TRAACT_CORE_EXPORT ComponentType {
    /**
     * invalid component, error, should not be used
     */
    INVALID = 0,
    /**
     * asynchronous data source component. use this for any component that provides data from some external source
     * e.g. camera, file, network to the traact dataflow network.
     *
     * Forcing the network to receive the data with AsyncSource can be done by running the component graph with SourceMode::WaitForBuffer
     *
     * Use:
     * auto buffer_future = request_callback_( timestamp ); // request buffer for timestamp, returns future with potential buffer
     * buffer_future.wait(); // wait for future, depending on the configuration for the time domain it will wait till a buffer is available or the timestamp is rejected
     * auto buffer_p = buffer_future.get(); // get buffer and write data into ports
     * buffer_p->Commit( UsefulData ); // after writing the data call Commit, parameter indicates whether the data written into the output can be used
     */
    ASYNC_SOURCE,
    /**
     * synchronous data source component. use this for module component outputs.
     * e.g. marker tracker as a module. one input module component receives the input image and calibration,
     * triggers the marker tracking in the module, module passes the output to the InternalSyncSource module output component
     * by using a callback set into the module. The advantage of this compared to using a SyncSource for the output is
     * that the output data can be passed directly into the component buffer. A SyncSource as the output would require
     * the component or module to buffer the data until "processTimePoint" is called. Without the need to buffer the data
     * the marker tracker can easily be written to use unlimited concurrency, enabling the ability for parallel execution
     * and reducing the dependencies of the resulting dataflow graph
     *
     * Use:
     * auto buffer_future = request_callback_( timestamp ); // request buffer for timestamp, returns future with potential buffer
     * buffer_future.wait(); // should return immediately since it was triggered by a already running component
     * auto buffer_p = buffer_future.get(); // get buffer and write data into ports
     * buffer_p->Commit( UsefulData ); // after writing the data call Commit, parameter indicates whether the data written into the output can be used
     */
    INTERNAL_SYNC_SOURCE,
    /**
     * synchronous data source component.
     * Using this component would means that the dataflow network requests
     * data for a specific timestamp (e.g. buffer, linear interpolation, kalman filtering (with potential different sync modes) ).
     * The functional component will be called at the right moment with no input and valid output to retrieve the data.
     *
     * Use:
     * Dataflow network calls: processTimePoint(Buffer& buffer)
     */
    SYNC_SOURCE,

    /**
     * standard use case for a traact component.
     * Define inputs and outputs to process input data and generate output data.
     * e.g. single marker tracker, feature extractor, image conversion, transformation if poses
     * Everything that should be processed in real time
     * Use:
     * Dataflow network calls: processTimePoint(Buffer& buffer)
     */
    SYNC_FUNCTIONAL,
    // not implemented yet
    ASYNC_FUNCTIONAL,
    /**
     * synchronous data sink component. use this for any component that provides data to programs outside of traact.
     * e.g. rendering of camera images and poses
     * Use:
     * Dataflow network calls: processTimePoint(Buffer& buffer)
     *
     * The buffer and its reuse is blocked by the call to processTimePoint. The function should return as fast as possible.
     */
    SYNC_SINK,
    // not implemented yet
    ASYNC_SINK

};

enum class ModuleType {
    INVALID = 0,
    GLOBAL,
    UNIQUE_DATAFLOW_PARAMETER,
    UNIQUE_NODE
};
}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_COMPONENTTYPES_H_

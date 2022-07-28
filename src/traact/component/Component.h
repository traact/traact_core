/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_COMPONENT_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_COMPONENT_H_

#include <string>
#include <functional>

#include <traact/component/ComponentTypes.h>
#include <traact/pattern/Pattern.h>
#include <traact/pattern/instance/PatternInstance.h>
#include <traact/datatypes.h>
#include <traact/traact_core_export.h>
#include <future>

namespace traact::buffer {
class SourceComponentBuffer;
class ComponentBuffer;
class ComponentBufferConfig;
}

namespace traact::component {

/**
 * Simple generic base for all traact components.
 * It contains empty implementations of every function.
 * Override as needed according to traact::component::ComponentType
 * and needs of component
 *
 */
class TRAACT_CORE_EXPORT Component {
 public:
    typedef typename std::shared_ptr<Component> Ptr;

    // used by source components
    typedef typename std::function<std::future<buffer::SourceComponentBuffer *>(Timestamp)> RequestCallback;
    typedef typename std::function<void(Timestamp, bool)> ReleaseAsyncCallback;
    typedef typename std::function<void(void)> SourceFinishedCallback;


    Component(std::string name);

    virtual ~Component() = default;

    /**
     * Create Pattern describing input and outputs of the component
     * @return
     */
    //static pattern::Pattern::Ptr GetPattern();

    /**
     * Name of component, unique in dataflow
     * @return
     */
    const std::string &getName() const;

    virtual void configureInstance(const pattern::instance::PatternInstance &pattern_instance);

    /**
     * Called once in the beginning after the dataflow network is constructed
     * but before the network starts
     * It can also be called during runtime between two data calls in case of changed
     * parameters
     *
     * Use to acquire devices, read configuration files and prepare component to be used.
     * initialization of buffers currently not supported
     *
     * @return false if initialization is not possible (e.g. camera not available)
     */
    virtual bool configure(const pattern::instance::PatternInstance &pattern_instance, buffer::ComponentBufferConfig *data);
    /**
     * Called after all components are initialized, the dataflow network is connected and ready to run
     *
     * @return false if component is not ready to be used
     */
    virtual bool start();

    /**
     * Request for component to stop operating.
     * Can be started again with by another call to start()
     * @return false if component can't stop
     */
    virtual bool stop();
    /**
     * Component is about to be destroyed
     * @return false if component failed to get into a state to be safely destroyed
     */
    virtual bool teardown();

    /**
     * Used by producing and consuming functional components and sink components.
     * Called when data for a new timestep ready to be processed.
     * All input buffers are filled and output buffers are ready to be written into
     * The component should return true when the processing succeeded and output buffers
     * are filled with new data for this timestamp
     *
     * @tparam data Provides input and output buffer to the component
     * @return false if output buffer are not in a meaningful state (VALID/INVALID) (e.g. exception), otherwise true
     */
    virtual bool processTimePoint(buffer::ComponentBuffer &data);

    /**
     * Same as processTimePoint but is called when at least one input port has the state INVALID
     * @param data Provides input and output buffer to the component
     * @return false if output buffer are not in a meaningful state (VALID/INVALID) (e.g. exception), otherwise true
     */
    virtual bool processTimePointWithInvalid(buffer::ComponentBuffer &data);

    /**
     * Used by source components.
     * In your thread or callback function when you get to the point that you have a
     * timestamp for the new data call request_callback_(ts) to register the new timestamp
     * in the dataflow network.
     *
     * Returns a SourceTimeDomainBuffer if successful and you can proceed to set the data.
     * In case of nullptr the dataflow network rejects this timestamp for some reason.
     * (examples Overloaded dataflow network, timestamp too old)
     * The reason should be reported by the network to the user.
     * In case of rejection you should throw away the measurement and try again with the next.
     *
     * @param commit_callback set by dataflow network
     */
    void setRequestCallback(const RequestCallback &request_callback);



    void setReleaseAsyncCallback(const ReleaseAsyncCallback &release_async_callback);

    void releaseAsyncCall(Timestamp timestamp, bool valid);

    void setRequestExitCallback(const SourceFinishedCallback &finished_callback);
    void setSourceFinished();

 protected:
    const std::string name_;

    // used by source components
    RequestCallback request_callback_{nullptr};
    ReleaseAsyncCallback release_async_callback_{nullptr};
    SourceFinishedCallback finished_callback_{nullptr};
};
}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_COMPONENT_H_

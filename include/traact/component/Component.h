/*  BSD 3-Clause License
 *
 *  Copyright (c) 2020, FriederPankratz <frieder.pankratz@gmail.com>
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#ifndef TRAACT_INCLUDE_TRAACT_COMPONENT_COMPONENT_H_
#define TRAACT_INCLUDE_TRAACT_COMPONENT_COMPONENT_H_

#include <string>
#include <functional>

#include <traact/component/ComponentTypes.h>
#include <traact/pattern/Pattern.h>
#include <traact/datatypes.h>
#include <traact/traact_core_export.h>
#include <rttr/type>

namespace traact::buffer {
    class TRAACT_CORE_EXPORT SourceTimeDomainBuffer;
    class TRAACT_CORE_EXPORT ComponentBuffer;
    class TRAACT_CORE_EXPORT ComponentBufferConfig;
}

namespace traact::component {

/**
 * Simple generic base for all traact components.
 * It contains empty implementations of every function.
 * Override as needed according to traact::component::ComponentType
 * and needs of component
 *
 * @tparam Buffer Provides input and output buffer to the user
 */
class TRAACT_CORE_EXPORT Component {
 public:
  typedef typename std::shared_ptr<Component> Ptr;

  // used by source components
  typedef typename std::function<buffer::SourceTimeDomainBuffer* (TimestampType)> RequestCallback;
    typedef typename std::function<void (TimestampType)> ReleaseAsyncCallback;


  Component(std::string name, const ComponentType traact_component_type);

  virtual ~Component() = default;

  /**
   * Create Pattern describing input and outputs of the component
   * @return
   */
  virtual pattern::Pattern::Ptr GetPattern() const = 0;

  /**
   * Name of component, unique in dataflow
   * @return
   */
  const std::string &getName() const;
  /**
   * Type of TraactNetwork supported type of component
   * @return
   */
  const ComponentType &getComponentType() const;


  /**
   * Called once in the beginning after the dataflow network is constructed
   * but before the network starts
   * It can also be called during runtime between two data calls in case of changed
   * parameters
   *
   * Use to acquire devices, read configuration files and prepare component to be used.
   * Also initialize buffers if necessary (e.g. gpu image buffers)
   *
   * @return false if initialization is not possible (e.g. camera not available)
   */
  virtual bool configure(const nlohmann::json &parameter, buffer::ComponentBufferConfig *data) ;
  /**
   * Called after all components are initialized, the dataflow network is connected and ready to run
   *
   * @return false if component is not ready to be used
   */
  virtual bool start();

  /**
   * Request for component to Stop operating.
   * Can be started again with by another call to Start()
   * @return false if component can't Stop
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
   * @param data
   * @return false if processing failed and output has not meaningful data
   */
  virtual bool processTimePoint(buffer::ComponentBuffer &data);

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

  virtual void invalidTimePoint(TimestampType ts, std::size_t mea_idx);

  void setReleaseAsyncCallback(const ReleaseAsyncCallback& releaseAsyncCallback);
  virtual void releaseAsyncCall(TimestampType ts);


  /* Enable RTTR Type Introspection */
  RTTR_ENABLE()

 protected:
  const std::string name_;
  const ComponentType traact_component_type_;

  // used by source components
  RequestCallback request_callback_{nullptr};
  ReleaseAsyncCallback releaseAsyncCallback_{nullptr};
};
}

#endif //TRAACT_INCLUDE_TRAACT_COMPONENT_COMPONENT_H_

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
#include <traact/traact_export.h>
namespace traact::buffer {
class TRAACT_EXPORT GenericComponentBuffer;
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
class TRAACT_EXPORT Component {
 public:
  typedef typename std::shared_ptr<Component> Ptr;

  // used by source components

  typedef typename std::function<int(TimestampType)> RequestCallbackType;
  typedef typename std::function<buffer::GenericComponentBuffer &(TimestampType)> AcquireCallbackType;
  typedef typename std::function<int(TimestampType)> CommitCallbackType;
  // used by producing and consuming functional components and sink components
  //typedef typename std::function<bool(Buffer &)> ProcessCallbackType;

  Component(std::string name, const ComponentType traact_component_type);

  virtual ~Component() = default;

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
   * Called once after the class is instantiated( before init())
   * and if the parameters change during runtime. If it is called
   * durring runtime then the user has to make sure that the call
   * is thread safe.
   * @param parameter
   */
  virtual void updateParameter(const nlohmann::json &parameter);

  /**
   * Called once in the beginning after the dataflow network is constructed
   * but before the connections are made or buffers are available.
   *
   * Use to acquire devices, read configuration files and prepare component to be used.
   *
   * @return false if initialization is not possible (e.g. camera not available)
   */
  virtual bool init();
  /**
   * Called after all components are initialized, the dataflow network is connected and ready to run
   *
   * @return false if component is not ready to be used
   */
  virtual bool start();

  /**
   * Request for component to stop operating.
   * Can be started again with by another call to Start()
   * @return false if component can't stop
   */
  virtual bool stop();
  /**
   * Component is about to be destroyed
   * @return false if component failed to get into a state to be safely destroyed
   */
  virtual bool teardown();

  /**
   * Initialize output buffers based on own information or input information.
   * The buffers themselves are not available yet.
   * Called after init, before start.
   * Can be called before a processTimePoint event in case the input meta info changed
   * @param data Buffer class for single component
   * @return false if initialization failed
   */
  virtual bool initializeBuffer(buffer::GenericComponentBuffer &data);

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
  virtual bool processTimePoint(buffer::GenericComponentBuffer &data);

  /**
   * Used by source components.
   * In your thread or callback function when you get to the point that you have a
   * timestamp for the new data call request_callback_(ts) to register the new timestamp
   * in the dataflow network.
   *
   * Return true if successful and you can proceed with acquire
   * In case of false the dataflow network rejects this timestamp for some reason.
   * (examples Overloaded dataflow network, timestamp too old)
   * The reason should be reported by the network to the user.
   * In case of rejection you should throw away the measurement and try again with the next.
   *
   * @param commit_callback set by dataflow network
   */
  void setRequestCallback(const RequestCallbackType &request_callback);
  /**
   * Used by source components.
   * Second step of sending new data into the network.
   * Use when request_callback returned true.
   * Call auto &buffer = acquire_callback_( timestamp )
   * Returns Buffer object to write data into.
   * Fill buffer with data.
   * It is best to immediately use the buffer memory, if possible, to avoid copy operations.
   *
   * @param commit_callback
   */
  void setAcquireCallback(const AcquireCallbackType &acquire_callback) ;

  /**
   * Used by source components.
   * When you are finished filling the buffer with data call commit_callback_
   * @param commit_callback
   */
  void setCommitCallback(const CommitCallbackType &commit_callback);

 protected:
  const std::string name_;
  const ComponentType traact_component_type_;

  // used by source components
  RequestCallbackType request_callback_;
  AcquireCallbackType acquire_callback_;
  CommitCallbackType commit_callback_;
};
}

#endif //TRAACT_INCLUDE_TRAACT_COMPONENT_COMPONENT_H_

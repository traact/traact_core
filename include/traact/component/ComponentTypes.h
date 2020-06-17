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

#ifndef TRAACT_INCLUDE_TRAACT_COMPONENT_COMPONENTTYPES_H_
#define TRAACT_INCLUDE_TRAACT_COMPONENT_COMPONENTTYPES_H_
#include <traact/traact_core_export.h>
namespace traact::component {
enum class TRAACT_CORE_EXPORT ComponentType {
  /**
   * invalid component, error, should not be used
   */
  Invalid = 0,
  /**
   * asynchronous data source component. use this for any component that provides data from some external source
   * e.g. camera, file, network to the traact dataflow network.
   *
   * Forcing the network to receive the data with AsyncSource can be done by running the component graph with SourceMode::WaitForBuffer
   *
   * Use:
   * request_callback_( timestamp ) : request buffer, if fail discard data and try with next timestamp, if succeed next:
   * auto &buffer = acquire_callback_ : get buffer, fill with data, next:
   * commit_callback_( timestamp ) : start processing
   */
  AsyncSource,
  /**
   * synchronous data source component.
   * Using this component would mean that the dataflow network would request
   * data for a specific timestamp (like a pull_function( timestamp ) ).
   * That would greatly hinder the dataflow and should not be used.
   * Requirements like the pull (e.g. buffer, linear interpolation, kalman filtering with different sync modes)
   * call are handled through a functional component with one arbitrary input.
   * The functional component will be called at the right moment with no valid input and valid output to retrieve the data.
   *
   * Forcing the network to receive the data with AsyncSource can be done by running the component graph with SourceMode::WaitForBuffer
   */
  //SyncSource,
  /**
   * synchronous data sink component. use this for any component that provides data to programs outside of traact.
   * e.g. Redering of camera images and poses
   * Use:
   * Dataflow network calls: processTimePoint(Buffer& buffer)
   *
   * The buffer and its reuse is blocked by the call to processTimePoint. The function should return as fast as possible.
   * Output will be reworked
   */
  SyncSink,
  /**
   * standard use case for a traact component.
   * Define inputs and outputs to process input data and generate output data.
   * e.g. marker tracker, feature extractor, image conversion, tranformation if poses
   * Everything that should be processed in real time
   * Use:
   * Dataflow network calls: processTimePoint(Buffer& buffer)
   */
  Functional,
  // not implemented yet
  /**
   * Same as Functional but for use cases where the data is not processed in real time (e.g. in a different thread).
   * e.g. bundle adjustment, any calibration
   * Will deep copy the input before passing it to the user
   * Use:
   * Dataflow network calls: processTimePoint(Buffer& buffer)
   */
  AsyncFunctional,
  AsyncSink, //problem images: copy output? otherwise buffer can not be reused. might be a good idea if ringbuffer is big enough. detach buffer elements?
  BufferedAsyncSink, // Buffer 1 timestamp (hold buffer until user retrieves), notify user of new data, have function to aquire buffer. use AsyncSource interface?

};

enum class ModuleType {
  Invalid = 0,
  Global,
  UniqueDataflowParameter,
  UniqueNode
};
}

#endif //TRAACT_INCLUDE_TRAACT_COMPONENT_COMPONENTTYPES_H_

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

#ifndef TRAACTMULTI_TRAACT_CORE_INCLUDE_TRAACT_UTIL_SEMAPHORE_H_
#define TRAACTMULTI_TRAACT_CORE_INCLUDE_TRAACT_UTIL_SEMAPHORE_H_

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <traact/datatypes.h>
namespace traact {
class Semaphore {
 public:
  Semaphore (int max_count=1, int count = 0)
      : max_count_(max_count), count_(count)
  {
  }

  inline void notify() {
    std::unique_lock<std::mutex> lock(mtx_);
    count_++;
    count_ = std::min(count_, max_count_);
    //notify the waiting thread
    cv_.notify_one();
  }

  inline void wait() {
    std::unique_lock<std::mutex> lock(mtx_);
    while(count_ == 0) {
      //wait on the mutex until notify is called
      cv_.wait(lock);
    }
    count_--;
  }

  inline int count() {
      std::unique_lock<std::mutex> lock(mtx_);
      return count_;
  }

 private:
  std::mutex mtx_;
  std::condition_variable cv_;
  int count_;
  int max_count_;
};

class WaitForMasterTs {

    public:
    WaitForMasterTs ()
                : current_ts(TimestampType::min())
        {
        }

        inline void notifyAll(const TimestampType ts) {
            std::unique_lock<std::mutex> lock(mtx_);
            current_ts = ts;
            cv_.notify_all();
        }

        inline void wait(const TimestampType ts) {
            std::unique_lock<std::mutex> lock(mtx_);
            while(current_ts > ts) {
                //wait on the mutex until notify is called
                cv_.wait(lock);
            }
        }


    private:
        std::mutex mtx_;
        std::condition_variable cv_;
        TimestampType current_ts;
    };

    class WaitForInit {

    public:
        WaitForInit ()
                : is_init(false)
        {
        }

        inline void SetInit(bool init) {
            std::unique_lock<std::mutex> lock(mtx_);
            is_init = init;
            cv_.notify_all();
        }

        inline void Wait() {
            std::unique_lock<std::mutex> lock(mtx_);
            while(!is_init) {
                //wait on the mutex until notify is called
                cv_.wait(lock);
            }
        }


    private:
        std::mutex mtx_;
        std::condition_variable cv_;
        bool is_init;
    };

    template <typename T>
    class WaitForValue {

    public:
        WaitForValue ()
                : current_value(std::numeric_limits<T>::min())
        {
        }

        inline void notifyAll(const T value) {
            std::unique_lock<std::mutex> lock(mtx_);
            current_value = value;
            cv_.notify_all();
        }

        inline void wait(const T value) {
            std::unique_lock<std::mutex> lock(mtx_);
            while(current_value > value) {
                //wait on the mutex until notify is called
                cv_.wait(lock);
            }
        }


    private:
        std::mutex mtx_;
        std::condition_variable cv_;
        T current_value;
    };

}


#endif //TRAACTMULTI_TRAACT_CORE_INCLUDE_TRAACT_UTIL_SEMAPHORE_H_

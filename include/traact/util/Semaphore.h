/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACTMULTI_TRAACT_CORE_INCLUDE_TRAACT_UTIL_SEMAPHORE_H_
#define TRAACTMULTI_TRAACT_CORE_INCLUDE_TRAACT_UTIL_SEMAPHORE_H_

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <traact/datatypes.h>
namespace traact {
class Semaphore {
 public:
    Semaphore (int max_count=1, int count = 0, TimeDuration timeout = std::chrono::milliseconds(100))
        : max_count_(max_count), count_(count), timeout_(timeout)
    {
    }

    inline void notify() {
        std::unique_lock<std::mutex> lock(mtx_);
        count_++;
        count_ = std::min(count_, max_count_);
        //notify the waiting thread
        cv_.notify_one();
    }

    inline bool wait() {
        std::unique_lock<std::mutex> lock(mtx_);
        while(count_ == 0) {
            //wait on the mutex until notify is called
            if(cv_.wait_for(lock, timeout_) == std::cv_status::timeout) {
                return false;
            }
        }
        count_--;
        return true;
    }

    inline bool try_wait(TimeDuration timeout = TimeDuration(0)) {
        std::unique_lock<std::mutex> lock(mtx_);
        while(count_ == 0) {
            //wait on the mutex until notify is called
            if(cv_.wait_for(lock, timeout) == std::cv_status::timeout) {
                return false;
            }
        }
        count_--;
        return true;
    }

    inline int count() {
        std::unique_lock<std::mutex> lock(mtx_);
        return count_;
    }

 private:
    int max_count_;
    int count_;
    TimeDuration timeout_;
    std::mutex mtx_;
    std::condition_variable cv_;


};

class WaitForMasterTs {

 public:
    WaitForMasterTs (std::chrono::milliseconds timeout = std::chrono::milliseconds(100))
        : timeout_(timeout), current_ts(kTimestampZero)
    {
    }

    inline void notifyAll(const Timestamp ts) {
        std::unique_lock<std::mutex> lock(mtx_);
        current_ts = ts;
        cv_.notify_all();
    }

    inline void wait(const Timestamp ts) {
        std::unique_lock<std::mutex> lock(mtx_);
        while(current_ts > ts) {
            //wait on the mutex until notify is called
            cv_.wait(lock);
        }
    }


 private:
    std::mutex mtx_;
    std::condition_variable cv_;
    std::chrono::milliseconds timeout_;
    Timestamp current_ts;
};

class WaitForInit {

 public:
    WaitForInit (std::chrono::milliseconds timeout = std::chrono::milliseconds(1000))
        : timeout_(timeout), is_init(false)
    {
    }

    inline void SetInit(bool init) {
        std::unique_lock<std::mutex> lock(mtx_);
        is_init = init;
        cv_.notify_all();
    }

    inline bool tryWait() {
        std::unique_lock<std::mutex> lock(mtx_);
        while(!is_init) {
            //wait on the mutex until notify is called
            if(cv_.wait_for(lock, timeout_) == std::cv_status::timeout) {
                return is_init;
            }
        }
        return is_init;
    }


 private:
    std::mutex mtx_;
    std::condition_variable cv_;
    std::chrono::milliseconds timeout_;
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
        while(current_value < value) {
            //wait on the mutex until notify is called
            cv_.wait(lock);
        }
    }

    inline T waitValue(const T value) {
        std::unique_lock<std::mutex> lock(mtx_);
        while(current_value < value) {
            //wait on the mutex until notify is called
            cv_.wait(lock);
        }
        return current_value;
    }

    inline T currentValue() const {
        return current_value;
    }


 private:
    std::mutex mtx_;
    std::condition_variable cv_;
    T current_value;
};

class WaitForTimestamp {

 public:
    WaitForTimestamp (TimeDuration max_offset)
        : current_value_(kTimestampZero), max_offset_(max_offset)
    {
    }

    inline void notifyAll(const Timestamp value) {
        std::unique_lock<std::mutex> lock(mtx_);
        current_value_ = value;
        cv_.notify_all();
    }

    inline void wait(const Timestamp value) {
        std::unique_lock<std::mutex> lock(mtx_);
        auto min_val = value - (max_offset_*2);

        while(current_value_ < min_val) {
            //wait on the mutex until notify is called
            cv_.wait(lock);
        }
    }

    inline Timestamp currentValue() const {
        return current_value_;
    }


 private:
    std::mutex mtx_;
    std::condition_variable cv_;
    Timestamp current_value_;
    TimeDuration max_offset_;
};

}

template <typename T>
class WaitForExactValue {

 public:
    WaitForExactValue ()
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
        while(current_value != value) {
            //wait on the mutex until notify is called
            cv_.wait(lock);
        }
    }

    inline bool try_wait(const T value, traact::TimeDuration timeout) {
        std::unique_lock<std::mutex> lock(mtx_);
        while(current_value != value) {
            //wait on the mutex until notify is called
            auto status = cv_.wait_for(lock, timeout);
            if(status == std::cv_status::timeout)
                return false;
        }
        return true;
    }



 private:
    std::mutex mtx_;
    std::condition_variable cv_;
    T current_value;
};


#endif //TRAACTMULTI_TRAACT_CORE_INCLUDE_TRAACT_UTIL_SEMAPHORE_H_

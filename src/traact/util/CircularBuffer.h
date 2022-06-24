/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_UTIL_CIRCULARBUFFER_H_
#define TRAACT_CORE_SRC_TRAACT_UTIL_CIRCULARBUFFER_H_

#include <cstdint>
#include <array>

namespace traact::util {

template<typename T, uint32_t N>
class CircularBuffer {
 public:
    void clear() {
        current_index_ = -1;
        current_size_ = 0;
    }

    void push_back(T value){
        ++current_index_;
        current_index_ = current_index_ % N;
        ++current_size_;
        current_size_ = std::min(current_size_, N);
        data_[current_index_] = value;
    }

    uint32_t size()  const{
        return current_size_;
    }
    T& at(uint32_t index){
        return data_[index];
    }
    const T& at(uint32_t index) const{
        return data_[index];
    }

    const T& front() const {
        auto index = current_index_ - (current_size_-1);
        if(index < 0){
            index += current_size_;
        }
        return data_[index];
    }

    const T& back() const {
        return data_[current_index_];
    }

 private:
    std::array<T, N> data_;
    uint32_t current_size_{0};
    int64_t current_index_{-1};


};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_UTIL_CIRCULARBUFFER_H_

/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_UTIL_CIRCULARBUFFER_H_
#define TRAACT_CORE_SRC_TRAACT_UTIL_CIRCULARBUFFER_H_

#include <cstdint>
#include <array>
#include <vector>
#include <utility>

namespace traact::util {

template<typename T, int N>
class CircularBuffer {
 public:
    using BufferType = std::array<T,N>;


    void clear() {
        current_index_ = -1;
        current_size_ = 0;
    }

    T& emplace_back(){
        ++current_index_;
        current_index_ = current_index_ % N;
        ++current_size_;
        current_size_ = std::min(current_size_, N);
        return back();
    }

    void push_back(T value){
        emplace_back();
        data_[current_index_] = value;
    }

    T& emplace_back(T&& value){
        emplace_back();
        data_[current_index_] = std::move(value);
        return back();
    }

    size_t size()  const{
        return current_size_;
    }
    T& at(size_t index){
        return data_[index];
    }
    const T& at(size_t index) const{
        return data_[index];
    }

    T& front() {
        return const_cast<T&>(std::as_const(*this).front());
    }

    T& back() {
        return const_cast<T&>(std::as_const(*this).back());
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

    T* begin() noexcept {
        return const_cast<T*>(static_cast<const CircularBuffer &>(*this).begin());
    }

    T* end() noexcept {
        return const_cast<T*>(static_cast<const CircularBuffer &>(*this).end());
    }

    const T* begin() const noexcept {
        if(current_size_ == 0) {
            return nullptr;
        } else {
            return data_.begin();
        }

    }

    const T* end() const noexcept {
        if(current_size_ == 0) {
            return nullptr;
        } else {
            return (data_.begin())+current_size_;
        }
    }

    T& operator[](size_t index){
        return data_[index];
    }

    const T& operator[](size_t index) const{
        return data_[index];
    }

 protected:
    std::array<T, N>  data_;
    int current_size_{0};
    int current_index_{-1};
};


template<typename T>
 class CircularDynamicBuffer  {
  public:
     using BufferType = std::vector<T>;

     CircularDynamicBuffer() = default;
     CircularDynamicBuffer(size_t size) {
         reserve(size);
     }

     void reserve(size_t size) {
         data_.resize(size);
     }
     void clear() {
         current_index_ = -1;
         current_size_ = 0;
     }

     void push_back(T value){
         ++current_index_;
         current_index_ = current_index_ % data_.size();
         ++current_size_;
         current_size_ = std::min(current_size_, static_cast<int>(data_.size()));
         data_[current_index_] = value;
     }

     void emplace_back(T&& value){
         ++current_index_;
         current_index_ = current_index_ % data_.size();
         ++current_size_;
         current_size_ = std::min(current_size_, static_cast<int>(data_.size()));
         data_[current_index_] = std::move(value);
     }

     int size()  const{
         return current_size_;
     }
     T& at(size_t index){
         return data_[index];
     }
     const T& at(size_t index) const{
         return data_[index];
     }

     T& front() {
         return const_cast<T&>(front());
     }

     T& back() {
         return const_cast<T&>(back());
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

     T* begin() noexcept {
         return const_cast<T*>(static_cast<const CircularDynamicBuffer &>(*this).begin());
     }

     T* end() noexcept {
         return const_cast<T*>(static_cast<const CircularDynamicBuffer &>(*this).end());
     }

     const T* begin() const noexcept {
         if(current_size_ == 0) {
             return nullptr;
         } else {
             return &front();
         }

     }

     const T* end() const noexcept {
         if(current_size_ == 0) {
             return nullptr;
         } else {
             return (&back())+1;
         }

     }

     T& operator[](size_t index){
         return data_[index];
     }

     const T& operator[](size_t index) const{
         return data_[index];
     }

  protected:
     std::vector<T>  data_;
     int current_size_{0};
     int current_index_{-1};

};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_UTIL_CIRCULARBUFFER_H_

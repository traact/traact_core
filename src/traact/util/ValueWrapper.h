/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_UTIL_VALUEWRAPPER_H_
#define TRAACT_CORE_SRC_TRAACT_UTIL_VALUEWRAPPER_H_

#include <memory>

#include "traact/traact_core_export.h"

namespace traact::util {

template<typename T>
class TRAACT_CORE_EXPORT ValueWrapper{
    public:
    ValueWrapper() = default;
    ValueWrapper(ValueWrapper const& image) = default;
    ValueWrapper& operator=(ValueWrapper const& image) =default;
    ValueWrapper(ValueWrapper && image) = default;
    ValueWrapper& operator=(ValueWrapper && image) = default;

    [[nodiscard]] const T& value() const{
        return value_;
    }

    [[nodiscard]] T& value() {
        return value_;
    }

    void update(T value){
        value_ = value;
    }

    template<typename TNative>
    void update(T value, std::shared_ptr<TNative> && owner){
        owner_ = std::move(owner);
        value_ = std::move(value);
    }

    private:
    T value_;
    std::shared_ptr<void> owner_{};

};
}

#endif //TRAACT_CORE_SRC_TRAACT_UTIL_VALUEWRAPPER_H_

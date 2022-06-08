/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_BUFFER_DATAFACTORY_H_
#define TRAACT_CORE_SRC_TRAACT_BUFFER_DATAFACTORY_H_

#include <string>
#include <memory>
#include <traact/traact_core_export.h>
#include <rttr/type>
namespace traact::buffer {

class TRAACT_CORE_EXPORT DataFactory : public std::enable_shared_from_this<DataFactory> {
 public:
    typedef typename std::shared_ptr<DataFactory> Ptr;
    DataFactory() = default;
    virtual ~DataFactory() = default;

    virtual std::string getTypeName() = 0;
    virtual void *createObject() = 0;
    virtual bool initObject(void *header, void *object) = 0;
    virtual void deleteObject(void *obj) = 0;

    template<typename Derived>
    std::shared_ptr<Derived> shared_from_base() {
        return std::static_pointer_cast<Derived>(shared_from_this());
    }
    /* Enable RTTR Type Introspection */
 RTTR_ENABLE()
};

/**
 * Default implementation of DataFactory calling new and delete
 * using the class defines MetaType and NativeType
 * @tparam T
 */
template<class T>
class TemplatedDefaultFactoryObject : public DataFactory {
 public:
    std::string getTypeName() override {
        return std::move(std::string(T::MetaType));
    }
    void *createObject() override {
        return new typename T::NativeType;
    }

    bool initObject(void *header, void *object) override {
        return true;
    }
    void deleteObject(void *obj) override {
        auto *tmp = static_cast<typename T::NativeType *>(obj);
        delete tmp;
    }

    /* Enable RTTR Type Introspection */
 RTTR_ENABLE(DataFactory)
};

}

#endif //TRAACT_CORE_SRC_TRAACT_BUFFER_DATAFACTORY_H_

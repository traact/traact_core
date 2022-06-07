/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_CORE_SRC_TRAACT_BUFFER_DATABUFFER_H_
#define TRAACT_CORE_SRC_TRAACT_BUFFER_DATABUFFER_H_

#include "DataFactory.h"
#include "BufferUtils.h"
#include <unordered_map>
namespace traact::buffer {

class DataBuffer {
 public:
    explicit DataBuffer(const std::set<DataFactory::Ptr> &data_factories);
    virtual ~DataBuffer();
    void *createObject(const std::string &type_name);
    void createObjects(const DataTypeNameVector &type_names);
    //bool initObject(void *header, void* object);
    void deleteAll();

    const std::vector<void *> &getData() const noexcept;
    const std::vector<std::string> &getTypes() const noexcept;

 private:
    std::map<std::string, DataFactory::Ptr> type_name_to_factory_;
    std::vector<void *> data_buffer_;
    std::vector<std::string> data_name_;
 RTTR_ENABLE()
};

using DataBufferPtr = std::unique_ptr<DataBuffer>;

} // traact::buffer

#endif //TRAACT_CORE_SRC_TRAACT_BUFFER_DATABUFFER_H_

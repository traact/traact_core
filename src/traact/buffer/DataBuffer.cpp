/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "DataBuffer.h"
#include <spdlog/spdlog.h>
namespace traact::buffer {
DataBuffer::DataBuffer(const std::set<DataFactory::Ptr> &data_factories) {
    for (const auto &factory : data_factories) {
        type_name_to_factory_.emplace(factory->getTypeName(), factory);
    }

}
DataBuffer::~DataBuffer() {
    deleteAll();
}
void *DataBuffer::createObject(const std::string &type_name) {
    auto factory = type_name_to_factory_.find(type_name);
    if (factory != type_name_to_factory_.end()) {
        auto *new_object = factory->second->createObject();
        auto* new_header = factory->second->createHeader();
        data_buffer_.emplace_back(new_object);
        header_buffer_.emplace_back(new_header);
        data_name_.emplace_back(type_name);
        return new_object;
    } else {
        SPDLOG_ERROR("Unknown type name {0}", type_name);
        return nullptr;
    }

}

void DataBuffer::deleteAll() {
    assert(data_buffer_.size() == data_name_.size());
    for (size_t i = 0; i < data_buffer_.size(); ++i) {
        type_name_to_factory_.at(data_name_[i])->deleteObject(data_buffer_[i]);
        type_name_to_factory_.at(data_name_[i])->deleteHeader(header_buffer_[i]);
    }
    data_buffer_.clear();
    data_name_.clear();
}
void DataBuffer::createObjects(const DataTypeNameVector &type_names) {
    data_buffer_.reserve(data_buffer_.size() + type_names.size());
    header_buffer_.reserve(data_buffer_.size() + type_names.size());
    data_name_.reserve(data_buffer_.size() + type_names.size());
    for (const auto &type_name : type_names) {
        createObject(type_name);
    }
}
const std::vector<void *> &DataBuffer::getData() const noexcept {
    return data_buffer_;
}
const std::vector<std::string> &DataBuffer::getTypes() const noexcept {
    return data_name_;
}
const std::vector<void *> &DataBuffer::getHeader() const noexcept {
    return header_buffer_;
}
} // traact::buffer
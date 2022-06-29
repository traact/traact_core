/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_BUFFER_TIMESTEPBUFFER_H_
#define TRAACT_CORE_SRC_TRAACT_BUFFER_TIMESTEPBUFFER_H_

#include <tuple>
#include <vector>
#include <map>
#include "ComponentBuffer.h"
#include "SourceComponentBuffer.h"
#include "traact/component/ComponentTypes.h"

namespace traact::buffer {


struct BufferConfig {
    component::ComponentType component_type;
    std::string instance_id;
    std::vector<std::pair<int, int>> buffer_to_port_input;
    std::vector<PortGroupConfig> port_group_config_input;
    std::vector<std::pair<int, int>> buffer_to_port_output;
    std::vector<PortGroupConfig> port_group_config_output;

};
using BufferType = std::vector<void *>;

class TimeStepBuffer {
 public:

    TimeStepBuffer(size_t time_step_index,
                   BufferType buffer_data,
                   BufferType header_data,
                   const std::map<int,
                                  BufferConfig> &buffer_config);
    size_t getComponentIndex(const std::string &component_name);
    ComponentBuffer &getComponentBuffer(size_t component_idx);
    ComponentBuffer &getComponentBuffer(const std::string &component_name);
    SourceComponentBuffer *getSourceComponentBuffer(size_t component_idx);
    std::future<bool> getSourceLock(size_t component_idx);
    void resetNewEvent();
    void setEvent(Timestamp timestamp, EventType message_type);
    void setEvent(Timestamp timestamp, EventType message_type, std::string changed_pattern);
    Timestamp getTimestamp();
    EventType getEventType();
    const std::string& getChangedPattern() const;

 private:
    const size_t time_step_index_;
    BufferType buffer_data_;
    BufferType header_data_;

    Timestamp current_ts_;
    EventType current_message_;
    std::map<std::string, size_t> component_buffer_to_index_;
    std::vector<std::unique_ptr<ComponentBuffer> > component_buffers_list_;
    std::vector<std::unique_ptr<SourceComponentBuffer> > source_buffer_list_;
    std::vector<std::unique_ptr<PortState > > buffer_valid_;
    //std::vector< PortState  > buffer_valid_;
    std::vector<Timestamp> buffer_timestamp_;
    std::string changed_pattern_instance_id_;

    std::unique_ptr<ComponentBuffer> createComponentBuffer(const BufferConfig &config, int component_index);

    void createLocalBuffer(const std::vector<std::pair<int, int>> &port_inputs,
                           LocalDataBuffer &data_buffer,
                           LocalHeaderBuffer &header_buffer,
                           LocalValidBuffer &valid_buffer,
                           LocalTimestampBuffer &timestamp_buffer);
};
}

#endif //TRAACT_CORE_SRC_TRAACT_BUFFER_TIMESTEPBUFFER_H_

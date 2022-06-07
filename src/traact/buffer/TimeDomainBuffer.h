/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_CORE_SRC_TRAACT_BUFFER_TIMEDOMAINBUFFER_H_
#define TRAACT_CORE_SRC_TRAACT_BUFFER_TIMEDOMAINBUFFER_H_

#include <atomic>
#include "traact/traact_core_export.h"
#include "TimeStepBuffer.h"
#include "DataFactory.h"
#include "traact/component/ComponentGraph.h"
#include "DataBufferFactory.h"
namespace traact::buffer {

class TRAACT_CORE_EXPORT TimeDomainBuffer {
 public:
    TimeDomainBuffer(int time_domain, std::shared_ptr<DataBufferFactory> buffer_factory);
    TimeDomainBuffer(TimeDomainBuffer &) = delete;
    TimeDomainBuffer &operator=(const TimeDomainBuffer &) = delete;
    TimeDomainBuffer(TimeDomainBuffer &&) = default;
    TimeDomainBuffer &operator=(TimeDomainBuffer &&) = default;

    ~TimeDomainBuffer() = default;

    void init(const component::ComponentGraph &value_a);

    TimeStepBuffer &getTimeStepBuffer(size_t concurrent_index);

    int getComponentIndex(const std::string &instance_id);
    [[nodiscard]] int getCountAsyncSources() const;
    [[nodiscard]] int getCountInternalSyncSources() const;
    [[nodiscard]] int getCountSources() const;

    void cancelAll();

 private:
    int time_domain_;
    int count_async_sources_{0};
    int count_internal_sync_sources_{0};
    int count_sources_{0};
    TimeDomainManagerConfig config_;
    std::shared_ptr<DataBufferFactory> buffer_factory_;
    std::vector<DataBufferPtr> data_buffer_;
    std::vector<std::unique_ptr<TimeStepBuffer>> time_step_buffer_;
    std::map<int, BufferConfig> buffer_config_;
    std::map<pattern::instance::ComponentID_PortName, size_t> port_to_buffer_index_;

    void cancelAllSources(int ringbuffer_index, bool valid);
};
}

#endif //TRAACT_CORE_SRC_TRAACT_BUFFER_TIMEDOMAINBUFFER_H_

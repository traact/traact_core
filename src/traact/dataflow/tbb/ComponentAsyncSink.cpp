/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "ComponentAsyncSink.h"

#include "NetworkGraph.h"

namespace traact::dataflow {
ComponentAsyncSink::ComponentAsyncSink(DefaultPatternPtr pattern_base,
                                       component::Component::Ptr component_base,
                                       TBBTimeDomainManager *buffer_manager,
                                       NetworkGraph *network_graph) : ComponentBase(std::move(pattern_base),
                                                                                    std::move(component_base),
                                                                                    std::move(buffer_manager),
                                                                                    network_graph) {
    node_ = nullptr;
    join_node_ = nullptr;
    sequencer_node_ = nullptr;

    this->component_base_->setReleaseAsyncCallback(std::bind(&ComponentAsyncSink::ReleaseGateway,
                                                             this,
                                                             std::placeholders::_1, std::placeholders::_2));
}

bool ComponentAsyncSink::init() {
    using namespace tbb::flow;

    ComponentBase::init();

    node_ = new AsyncNodeType(this->network_graph_->getTBBGraph(),
                              this->pattern_base_->getConcurrency(),
                              std::bind(&ComponentAsyncSink::submit,
                                        this,
                                        std::placeholders::_1,
                                        std::placeholders::_2));

    int count_input = this->pattern_base_->getConsumerPorts().size();
    if (count_input > 1) {
        join_node_ = new DynamicJoinNode(this->network_graph_->getTBBGraph(), count_input);

    }

    if (pattern_base_->getConcurrency() != unlimited) {
        sequencer_node_ =
            new sequencer_node<TraactMessage>(network_graph_->getTBBGraph(), [](const TraactMessage &msg) -> size_t {
                return msg.event_idx;
            });

        if (count_input > 1) {
            make_edge(join_node_->getSender(), *sequencer_node_);
        }

        make_edge(*sequencer_node_, *node_);

    } else {
        if (count_input > 1) {
            make_edge(join_node_->getSender(), *node_);
        }
    }

    return true;
}

bool ComponentAsyncSink::teardown() {
    ComponentBase::teardown();

    delete node_;
    delete join_node_;
    delete sequencer_node_;

    return true;
}

void ComponentAsyncSink::submit(TraactMessage in, AsyncNodeType::gateway_type &gateway) {
    gateway.reserve_wait();
    try {

        DefaultComponentBuffer
            &component_buffer = in.domain_buffer->GetComponentBuffer(component_base_->getName());

        SPDLOG_TRACE("Component {0}; ts {1}; {2}", component_base_->getName(),
                     component_buffer.GetTimestamp().time_since_epoch().count(), in.toString());
        switch (in.message_type) {
            case MessageType::Configure: {
                bool configure_result = component_base_->configure(pattern_base_->local_pattern.parameter,
                                                                   nullptr);
                SPDLOG_INFO("{0}: configure {1}", GetName(), configure_result);
                break;
            }
            case MessageType::Start: {
                component_base_->start();
                break;
            }
            case MessageType::Stop: {
                component_base_->stop();
                break;
            }
            case MessageType::Teardown: {
                component_base_->teardown();
                break;
            }

            case MessageType::Data: {
                work_type w = {in, &gateway};
                async_messages_.emplace(component_buffer.GetTimestamp(), w);

                if (in.valid_data) {
                    this->component_base_->processTimePoint(component_buffer);
                } else {
                    SPDLOG_TRACE("input for component not valid, skip processing: {0}; MeaIndex: {1}",
                                 component_base_->getName(), in.event_idx);
                    component_base_->invalidTimePoint(component_buffer.GetTimestamp(), in.event_idx);
                }
                return;
            }
            case MessageType::Invalid:
            default: {
                SPDLOG_ERROR("Component {0}; idx {1}; {2}", component_base_->getName(), in.event_idx,
                             "invalid message");
                break;
            }

        }

    } catch (...) {
        SPDLOG_ERROR("unknown exception in {0}", component_base_->getName());
        in.valid_data = false;
    }
    gateway.try_put(in);
    gateway.release_wait();

}
void ComponentAsyncSink::connect() {
    auto tmp = this->pattern_base_->getConsumerPorts();

    for (auto port : tmp) {
        for (auto producer_port : port->connectedToPtr()) {
            tbb::flow::make_edge(network_graph_->getSender(producer_port), getReceiver(port->getPortIndex()));
        }
    }
}
void ComponentAsyncSink::disconnect() {

}

tbb::flow::receiver<TraactMessage> &ComponentAsyncSink::getReceiver(int index) {
    using namespace tbb::flow;

    if (pattern_base_->getConcurrency() != unlimited) {
        if (join_node_ == nullptr) {
            return *sequencer_node_;
        } else {
            return join_node_->getReceiver(index);
        }
    } else {
        if (join_node_ == nullptr) {
            return *node_;
        } else {
            return join_node_->getReceiver(index);
        }
    };

}

component::ComponentType ComponentAsyncSink::getComponentType() {
    return component::ComponentType::AsyncSink;
}

tbb::flow::sender<TraactMessage> &ComponentAsyncSink::getSender(int index) {
    return *node_;
}

void ComponentAsyncSink::ReleaseGateway(TimestampType ts, bool valid) {
    MapDataType::accessor result;
    async_messages_.find(result, ts);
    if (!result.empty()) {
        result->second.input.valid_data = valid;
        result->second.gateway->try_put(result->second.input);
        result->second.gateway->release_wait();
        async_messages_.erase(result);
    } else {
        SPDLOG_ERROR("{0}: ReleaseGateway timestamp {1} not found", GetName(), ts.time_since_epoch().count());
    }
}
}
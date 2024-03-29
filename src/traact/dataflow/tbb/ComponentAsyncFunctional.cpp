/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "ComponentAsyncFunctional.h"
#include "NetworkGraph.h"

namespace traact::dataflow {

ComponentAsyncFunctional::ComponentAsyncFunctional(DefaultPatternPtr pattern_base,
                                                   component::Component::Ptr component_base,
                                                   TBBTimeDomainManager *buffer_manager,
                                                   NetworkGraph *network_graph) : ComponentBase(std::move(
                                                                                                    pattern_base),
                                                                                                std::move(
                                                                                                    component_base),
                                                                                                std::move(
                                                                                                    buffer_manager),
                                                                                                network_graph),
                                                                                  node_(
                                                                                      nullptr),
                                                                                  join_node_(nullptr) {
    this->component_base_->setReleaseAsyncCallback(std::bind(&ComponentAsyncFunctional::ReleaseGateway,
                                                             this,
                                                             std::placeholders::_1, std::placeholders::_2));
}

bool ComponentAsyncFunctional::init() {
    using namespace tbb::flow;

    ComponentBase::init();

    node_ = new AsyncNodeType(this->network_graph_->getTBBGraph(),
                              this->pattern_base_->getConcurrency(),
                              std::bind(&ComponentAsyncFunctional::submit,
                                        this,
                                        std::placeholders::_1,
                                        std::placeholders::_2));

    int count_input = this->pattern_base_->getConsumerPorts().size();
    if (count_input > 1) {
        join_node_ = new DynamicJoinNode(this->network_graph_->getTBBGraph(), count_input);
        make_edge(join_node_->getSender(), *node_);

    }

    broadcast_node_ = new broadcast_node<TraactMessage>(this->network_graph_->getTBBGraph());
    make_edge(*node_, *broadcast_node_);

    return true;
}

bool ComponentAsyncFunctional::teardown() {
    ComponentBase::teardown();

    delete node_;
    if (join_node_)
        delete join_node_;

    return true;
}

void ComponentAsyncFunctional::connect() {
    auto tmp = this->pattern_base_->getConsumerPorts();
    for (auto port : tmp) {
        for (auto producer_port : port->connectedToPtr()) {
            tbb::flow::make_edge(this->network_graph_->getSender(producer_port), getReceiver(port->getPortIndex()));
        }
    }
}

void ComponentAsyncFunctional::disconnect() {

}

tbb::flow::receiver<TraactMessage> &ComponentAsyncFunctional::getReceiver(int index) {
    using namespace tbb::flow;

    if (join_node_ == nullptr) {
        return *node_;
    } else {
        return join_node_->getReceiver(index);
    }
}
tbb::flow::sender<TraactMessage> &ComponentAsyncFunctional::getSender(int index) {
    return *broadcast_node_;
}

component::ComponentType ComponentAsyncFunctional::getComponentType() {
    return component::ComponentType::AsyncFunctional;
}

void ComponentAsyncFunctional::ReleaseGateway(TimestampType ts, bool valid) {
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

void ComponentAsyncFunctional::submit(TraactMessage in,
                                      tbb::flow::async_node<TraactMessage, TraactMessage>::gateway_type &gateway) {
    gateway.reserve_wait();
    try {

        auto &component_buffer = in.domain_buffer->GetComponentBuffer(component_base_->getName());

        SPDLOG_TRACE("Component {0}; ts {1}; {2}",
                     component_base_->getName(),
                     component_buffer.GetTimestamp().time_since_epoch().count(),
                     in.toString());

        switch (in.message_type) {
            case MessageType::Configure: {
                bool configure_result = component_base_->configure(pattern_base_->local_pattern.parameter, nullptr);
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
                                 component_base_->getName(),
                                 in.event_idx);
                    component_base_->invalidTimePoint(component_buffer.GetTimestamp(), in.event_idx);
                }
                return;
            }

            case MessageType::Invalid:
            default: {
                SPDLOG_ERROR("Component {0}; idx {1}; {2}",
                             component_base_->getName(),
                             in.event_idx,
                             "invalid message");
                break;
            }

        }


        //component_buffer.commit();


    } catch (...) {
        SPDLOG_ERROR("unknown exception in {0}", component_base_->getName());
        in.valid_data = false;
    }

    gateway.try_put(in);
    gateway.release_wait();

}

}
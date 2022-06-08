/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "ComponentSyncSource.h"
#include "NetworkGraph.h"

namespace traact::dataflow {

ComponentSyncSource::ComponentSyncSource(DefaultPatternPtr pattern_base,
                                         DefaultComponentPtr component_base,
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

    //buffer_manager_->RegisterBufferSource(this);
}

bool ComponentSyncSource::init() {
    using namespace tbb::flow;

    ComponentBase::init();

    node_ = new function_node<TraactMessage, TraactMessage>(this->network_graph_->getTBBGraph(),
                                                            this->pattern_base_->getConcurrency(),
                                                            *this);

//        int count_input = this->pattern_base_->getConsumerPorts().size();
//        if (count_input > 1) {
//            join_node_ = new DynamicJoinNode(this->network_graph_->getTBBGraph(), count_input);
//            make_edge(join_node_->getSender(), *node_);
//
//        }

    broadcast_node_ = new broadcast_node<TraactMessage>(this->network_graph_->getTBBGraph());
    make_edge(*node_, *broadcast_node_);

    return true;
}

bool ComponentSyncSource::teardown() {
    ComponentBase::teardown();

    delete node_;
//        if(join_node_)
//            delete join_node_;

    return true;
}

TraactMessage ComponentSyncSource::operator()(const TraactMessage &in) {
    TraactMessage result = in;

    try {

        DefaultComponentBuffer &component_buffer = in.domain_buffer->GetComponentBuffer(component_base_->getName());

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
                //component_base_->start();
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
                if (in.valid_data) {
                    SPDLOG_TRACE("input for component valid, processing: {0}; MeaIndex: {1}",
                                  component_base_->getName(),
                                  in.event_idx);
                    result.valid_data = component_base_->processTimePoint(component_buffer);
                } else {
                    SPDLOG_TRACE("input for component not valid, skip processing: {0}; MeaIndex: {1}",
                                  component_base_->getName(),
                                  in.event_idx);
                    component_base_->invalidTimePoint(component_buffer.GetTimestamp(), in.event_idx);
                }

                break;
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

    } catch (...) {
        SPDLOG_ERROR("unknown exception in {0}", component_base_->getName());
        result.valid_data = false;
    }
    return result;
}

void ComponentSyncSource::connect() {
    return;
//        auto tmp = this->pattern_base_->getConsumerPorts();
//        for (auto port : tmp) {
//            for (auto producer_port : port->connectedToPtr()) {
//                tbb::flow::make_edge(this->network_graph_->getSender(producer_port), getReceiver(port->getPortIndex()));
//            }
//        }
}

void ComponentSyncSource::disconnect() {

}

tbb::flow::receiver<TraactMessage> &ComponentSyncSource::getReceiver(int index) {
    using namespace tbb::flow;
//
//        if (join_node_ == nullptr) {
//            return *node_;
//        } else {
//            return join_node_->getReceiver(index);
//        }

    return *node_;
}
tbb::flow::sender<TraactMessage> &ComponentSyncSource::getSender(int index) {
    return *broadcast_node_;
}

component::ComponentType ComponentSyncSource::getComponentType() {
    return component::ComponentType::SyncSource;
}

std::string ComponentSyncSource::getComponentName() {
    return component_base_->getName();
}

}
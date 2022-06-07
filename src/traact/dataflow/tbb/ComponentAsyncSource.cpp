/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#include "ComponentAsyncSource.h"

#include "NetworkGraph.h"

namespace traact::dataflow {
ComponentAsyncSource::ComponentAsyncSource(DefaultPatternPtr pattern_base,
                                           DefaultComponentPtr component_base,
                                           TBBTimeDomainManager *buffer_manager,
                                           NetworkGraph *network_graph) : ComponentBase(
    std::move(pattern_base),
    std::move(component_base),
    std::move(buffer_manager),
    network_graph)//, node_(nullptr)
{

    this->component_base_->setRequestCallback(std::bind(&TBBTimeDomainManager::RequestSourceBuffer,
                                                        buffer_manager,
                                                        std::placeholders::_1, component_index_));


    //buffer_manager_->RegisterBufferSource(this);

}

bool ComponentAsyncSource::init() {
    ComponentBase::init();
    using namespace tbb::flow;

    node_ = new function_node<TraactMessage, TraactMessage>(this->network_graph_->getTBBGraph(),
                                                            this->pattern_base_->getConcurrency(),
                                                            *this);

    broadcast_node_ = new broadcast_node<TraactMessage>(this->network_graph_->getTBBGraph());
    make_edge(*node_, *broadcast_node_);

    return true;
}

bool ComponentAsyncSource::start() {
    return ComponentBase::start();
}

bool ComponentAsyncSource::stop() {
    ComponentBase::stop();

    return true;
}

bool ComponentAsyncSource::teardown() {
    ComponentBase::teardown();

    delete broadcast_node_;

    return true;
}

tbb::flow::sender<TraactMessage> &ComponentAsyncSource::getSender(int index) {
    return *broadcast_node_;
}

tbb::flow::receiver<TraactMessage> &ComponentAsyncSource::getReceiver(int index) {
    return *node_;
}

void ComponentAsyncSource::connect() {

}

void ComponentAsyncSource::disconnect() {

}

std::string ComponentAsyncSource::getComponentName() {
    return component_base_->getName();
}

component::ComponentType ComponentAsyncSource::getComponentType() {
    return component::ComponentType::AsyncSource;
}

TraactMessage ComponentAsyncSource::operator()(const TraactMessage &in) {
    TraactMessage result = in;
    try {


        //DefaultComponentBuffer &component_buffer = in.domain_buffer->getComponentBuffer(component_base_->getName());

        SPDLOG_TRACE("ComponentAsyncSource {0}: {1}", component_base_->getName(), in.toString());

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

}

